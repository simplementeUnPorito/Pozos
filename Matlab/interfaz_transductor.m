function interfaz_transductor()
% INTERFAZ_TRANSDUCTOR  Adquisición y guardado para transductor piezoeléctrico
%
% Recibe LONGITUD=2000 floats en ASCII via UART del PSoC (CY8CKIT-059).
% Guarda cada medición en un struct array dentro de un único archivo .mat.
%
% ── PROTOCOLO SERIAL (ASCII, un float por línea) ──────────────────────────
%   S<LF>          ← inicio de trama  (opcional)
%   1.2345<LF>     ← LONGITUD floats, uno por línea
%   ...
%   E<LF>          ← fin de trama  (se cierra solo al llegar a LONGITUD)
%
%   Si el PSoC no envía marcadores S/E, la trama se detecta automáticamente
%   al recibir LONGITUD valores numéricos consecutivos.
%   El botón DISPARAR envía el byte 'T' al PSoC para solicitar adquisición.
%
% ── ESTRUCTURA GUARDADA EN .mat ────────────────────────────────────────────
%   datos(i).pilote       string  — identificador del pilote
%   datos(i).profundidad  double  — profundidad [m]
%   datos(i).stickup      double  — stickup del pivote [m]
%   datos(i).senal        Nx1     — señal ADC convertida a voltios
%   datos(i).tiempo       Nx1     — eje de tiempo [ms]
%   datos(i).fs           double  — frecuencia de muestreo [Hz]
%   datos(i).fecha        string  — timestamp de adquisición
%   datos(i).n_muestras   int     — cantidad de muestras recibidas
%
% ── LÓGICA DE GUARDADO ─────────────────────────────────────────────────────
%   Tags iguales  (pilote + profundidad + stickup) → sobreescribe
%   Tags distintos                                 → append
%
% Requiere MATLAB R2019b+ (serialport API).
% ──────────────────────────────────────────────────────────────────────────

% =========================================================================
% Constantes
% =========================================================================
LONGITUD      = 2000;
BAUD_VALS     = [9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600];
BAUD_STRS     = {'9600','19200','38400','57600','115200','230400','460800','921600'};
DEF_BAUD_IDX  = 5;   % 115200

% Paleta de colores
C_BG  = [0.93 0.93 0.93];
C_GRN = [0.15 0.65 0.20];
C_RED = [0.82 0.18 0.14];
C_BLU = [0.12 0.40 0.82];
C_ORG = [0.88 0.52 0.08];
C_WHT = [1.00 1.00 1.00];

% =========================================================================
% Figura principal
% =========================================================================
fig = figure( ...
    'Name',            'Interfaz Transductor de Presión – Pozos', ...
    'NumberTitle',     'off', ...
    'Position',        [30 30 1260 830], ...
    'Resize',          'on', ...
    'Color',           C_BG, ...
    'CloseRequestFcn', @onClose);

% =========================================================================
% Estado de la aplicación  (compartido mediante guidata)
% =========================================================================
app.LONGITUD   = LONGITUD;
app.serial_obj = [];
app.connected  = false;
app.receiving  = false;
app.in_frame   = false;
app.rx_floats  = zeros(1, LONGITUD);
app.rx_count   = 0;
app.cur_signal = [];
app.cur_time   = [];
app.mat_file   = 'datos_pozos.mat';
app.pilotes    = [];

% =========================================================================
% ── Panel 1: Conexión Serial ─────────────────────────  arriba-izquierda
% =========================================================================
pp = mkpanel(fig, ' Conexión Serial', [0.005 0.775 0.235 0.215]);

mktext(pp, 'Puerto:',    [0.04 0.72 0.38 0.18]);
mktext(pp, 'Baud rate:', [0.04 0.46 0.38 0.18]);

ui.edit_port = mkedit(pp, 'COM3',   [0.43 0.72 0.54 0.18]);
ui.pop_baud  = uicontrol(pp, 'Style', 'popupmenu', ...
    'String', BAUD_STRS, 'Value', DEF_BAUD_IDX, ...
    'Units', 'normalized', 'Position', [0.43 0.46 0.54 0.18]);

ui.btn_connect = mkbtn(pp, 'CONECTAR', [0.06 0.20 0.88 0.21], C_GRN, @onConnectBtn);

ui.lbl_conn = uicontrol(pp, 'Style', 'text', ...
    'String', 'Desconectado', ...
    'Units', 'normalized', 'Position', [0.04 0.01 0.92 0.17], ...
    'FontWeight', 'bold', 'HorizontalAlignment', 'center', ...
    'ForegroundColor', C_RED, 'BackgroundColor', C_BG);

% =========================================================================
% ── Panel 2: Metadata ────────────────────────────────  arriba-centro
% =========================================================================
pp = mkpanel(fig, ' Metadata de Medición', [0.248 0.775 0.455 0.215]);

% ── Fila 1: Gestión de Pilotes (prominente) ────────────────────────────
mktext(pp, 'Pilote:',      [0.02 0.65 0.18 0.22]);
ui.pop_pilote = uicontrol(pp, 'Style', 'popupmenu', ...
    'String', {'(sin pilotes)'}, 'Value', 1, ...
    'Units', 'normalized', 'Position', [0.21 0.65 0.38 0.22], ...
    'Callback', @onPiloteChanged, 'FontSize', 10);
ui.btn_new_pilote = mkbtn(pp, '⚙ GESTIONAR\nPILOTES', [0.61 0.63 0.37 0.26], C_ORG, @onNewPilote);

% ── Fila 2: Profundidad, Stickup ────────────────────────────────────────
mktext(pp, 'Profundidad:', [0.02 0.40 0.18 0.18]);
ui.edit_prof = mkedit(pp, '0.0', [0.21 0.41 0.15 0.19]);
mktext(pp, 'm', [0.37 0.41 0.04 0.18]);

mktext(pp, 'Stickup:',     [0.52 0.40 0.18 0.18]);
ui.lbl_stickup_val = uicontrol(pp, 'Style', 'text', 'String', '—', ...
    'Units', 'normalized', 'Position', [0.71 0.41 0.15 0.19], ...
    'HorizontalAlignment', 'center', 'FontWeight', 'bold', 'FontSize', 10, ...
    'BackgroundColor', [0.95 0.95 0.95]);
mktext(pp, 'm', [0.87 0.41 0.04 0.18]);

% ── Fila 3: Frec. muestreo, Archivo .mat ────────────────────────────────
mktext(pp, 'Frec. (Hz):', [0.02 0.15 0.18 0.18]);
ui.edit_fs = mkedit(pp, '10000', [0.21 0.16 0.15 0.19]);

mktext(pp, 'Archivo:',     [0.52 0.15 0.15 0.18]);
ui.edit_matfile = mkedit(pp, 'datos_pozos.mat', [0.68 0.16 0.22 0.19]);
mkbtn(pp, '...', [0.91 0.16 0.07 0.19], C_BG(1,:), @onBrowse);

% Info
ui.lbl_nrec = uicontrol(pp, 'Style', 'text', ...
    'String', 'Registros: 0', ...
    'Units', 'normalized', 'Position', [0.02 0.00 0.96 0.12], ...
    'FontWeight', 'bold', 'HorizontalAlignment', 'left', ...
    'BackgroundColor', C_BG);

% =========================================================================
% ── Panel 3: Control de Adquisición ─────────────────  arriba-derecha
% =========================================================================
pp = mkpanel(fig, ' Control de Adquisición', [0.712 0.775 0.283 0.215]);

ui.btn_trigger = mkbtn(pp, sprintf('DISPARAR\nADQUISICIÓN'), ...
    [0.04 0.42 0.45 0.52], C_BLU, @onTrigger);
ui.btn_save    = mkbtn(pp, sprintf('GUARDAR\n  SEÑAL'), ...
    [0.53 0.42 0.43 0.52], C_ORG, @onSave);
set([ui.btn_trigger ui.btn_save], 'Enable', 'off');

ui.lbl_status = uicontrol(pp, 'Style', 'text', ...
    'String', 'Estado: Desconectado', ...
    'Units', 'normalized', 'Position', [0.03 0.22 0.94 0.18], ...
    'FontWeight', 'bold', 'HorizontalAlignment', 'left', ...
    'BackgroundColor', C_BG);

ui.lbl_samples = uicontrol(pp, 'Style', 'text', ...
    'String', 'Muestras: —', ...
    'Units', 'normalized', 'Position', [0.03 0.03 0.94 0.17], ...
    'HorizontalAlignment', 'left', 'BackgroundColor', C_BG);

% =========================================================================
% ── Panel 4: Plot principal ──────────────────────────  centro-izquierda
% =========================================================================
pp = mkpanel(fig, ' Señal Adquirida', [0.005 0.225 0.70 0.545]);

ui.ax = axes('Parent', pp, ...
    'Units', 'normalized', 'Position', [0.09 0.13 0.88 0.82]);
xlabel(ui.ax, 'Tiempo (ms)', 'FontSize', 10);
ylabel(ui.ax, 'Amplitud (V)', 'FontSize', 10);
title(ui.ax, 'Sin datos — conectar y disparar adquisición', 'FontSize', 10);
grid(ui.ax, 'on');  box(ui.ax, 'on');
ui.hline = plot(ui.ax, NaN, NaN, 'b-', 'LineWidth', 1.3);

% =========================================================================
% ── Panel 5: Estadísticas de señal ──────────────────  centro-derecha-top
% =========================================================================
pp = mkpanel(fig, ' Estadísticas', [0.713 0.530 0.282 0.240]);

ypos = [0.76 0.60 0.44 0.28 0.12];
mktext(pp, 'Máx (V):',       [0.04 ypos(1) 0.52 0.16]);
mktext(pp, 'Mín (V):',       [0.04 ypos(2) 0.52 0.16]);
mktext(pp, 'Media (V):',     [0.04 ypos(3) 0.52 0.16]);
mktext(pp, 'RMS (V):',       [0.04 ypos(4) 0.52 0.16]);
mktext(pp, 'Duración (ms):', [0.04 ypos(5) 0.52 0.16]);
ui.lbl_max  = mkvaltext(pp, '—', [0.57 ypos(1) 0.40 0.16]);
ui.lbl_min  = mkvaltext(pp, '—', [0.57 ypos(2) 0.40 0.16]);
ui.lbl_mean = mkvaltext(pp, '—', [0.57 ypos(3) 0.40 0.16]);
ui.lbl_rms  = mkvaltext(pp, '—', [0.57 ypos(4) 0.40 0.16]);
ui.lbl_dur  = mkvaltext(pp, '—', [0.57 ypos(5) 0.40 0.16]);

% =========================================================================
% ── Panel 6: Ventana de tiempo (zoom) ───────────────  abajo-izquierda
% =========================================================================
pp = mkpanel(fig, ' Ventana de Tiempo', [0.005 0.010 0.295 0.208]);

mktext(pp, 'Desde (ms):', [0.04 0.66 0.44 0.22]);
mktext(pp, 'Hasta (ms):', [0.04 0.36 0.44 0.22]);

ui.edit_t0 = mkedit(pp, '0',   [0.50 0.66 0.46 0.22]);
ui.edit_t1 = mkedit(pp, '200', [0.50 0.36 0.46 0.22]);

mkbtn(pp, 'Aplicar Zoom', [0.04 0.06 0.44 0.26], C_BG(1,:), @onZoomApply);
mkbtn(pp, 'Reset Zoom',   [0.52 0.06 0.44 0.26], C_BG(1,:), @onZoomReset);

% =========================================================================
% ── Panel 7: Registros guardados ────────────────────  derecha-completo
% =========================================================================
pp = mkpanel(fig, ' Registros en .mat', [0.713 0.010 0.282 0.512]);

ui.list_rec = uicontrol(pp, 'Style', 'listbox', ...
    'Units', 'normalized', 'Position', [0.02 0.14 0.96 0.84], ...
    'String', {}, 'FontSize', 8, 'BackgroundColor', C_WHT);

mkbtn(pp, 'Cargar seleccionado', [0.02 0.01 0.96 0.12], C_BG(1,:), @onLoadRecord);

% =========================================================================
% ── Panel 8: Protocolo (info) ───────────────────────  abajo-centro
% =========================================================================
pp = mkpanel(fig, ' Protocolo', [0.310 0.010 0.395 0.208]);

proto_str = sprintf(['Protocolo ASCII esperado:\n' ...
    '  S<LF>          ← inicio de trama\n' ...
    '  1.2345<LF>     ← %d floats, uno por línea\n' ...
    '  E<LF>          ← fin de trama\n\n' ...
    'Trigger: envía ''T'' por serial.\n' ...
    'Sin marcadores S/E se detecta automáticamente.'], LONGITUD);
uicontrol(pp, 'Style', 'text', 'String', proto_str, ...
    'Units', 'normalized', 'Position', [0.02 0.02 0.96 0.96], ...
    'HorizontalAlignment', 'left', 'FontSize', 8.5, 'BackgroundColor', C_BG);

% =========================================================================
% Guardar estado
% =========================================================================
app.ui = ui;
guidata(fig, app);
refreshPilotesList();
refreshRecordsList();

% #########################################################################
% CALLBACKS
% #########################################################################

% ── Conectar / Desconectar ───────────────────────────────────────────────
    function onConnectBtn(~, ~)
        app = guidata(fig);
        if app.connected
            doDisconnect();
        else
            doConnect();
        end
    end

    function doConnect()
        app  = guidata(fig);
        port = strtrim(get(app.ui.edit_port, 'String'));
        baud = BAUD_VALS(get(app.ui.pop_baud, 'Value'));
        try
            s = serialport(port, baud, 'Timeout', 2);
            configureTerminator(s, 'LF');
            flush(s);
            configureCallback(s, 'terminator', @serialRxCb);

            app.serial_obj = s;
            app.connected  = true;
            app.rx_count   = 0;
            app.in_frame   = false;
            app.receiving  = false;

            set(app.ui.btn_connect, 'String', 'DESCONECTAR', 'BackgroundColor', C_RED);
            set(app.ui.lbl_conn,    'String', ['Conectado: ' port], ...
                'ForegroundColor', [0 0.55 0.10]);
            set(app.ui.btn_trigger, 'Enable', 'on');
            set(app.ui.lbl_status,  'String', 'Estado: Conectado — Listo', ...
                'ForegroundColor', [0 0 0]);
            guidata(fig, app);
        catch err
            errordlg(sprintf('No se pudo abrir %s:\n%s', port, err.message), ...
                'Error de Conexión');
        end
    end

    function doDisconnect()
        app = guidata(fig);
        try
            if ~isempty(app.serial_obj) && isvalid(app.serial_obj)
                configureCallback(app.serial_obj, 'off');
                delete(app.serial_obj);
            end
        catch
        end

        app.serial_obj = [];
        app.connected  = false;
        app.receiving  = false;
        app.in_frame   = false;
        app.rx_count   = 0;

        set(app.ui.btn_connect, 'String', 'CONECTAR',     'BackgroundColor', C_GRN);
        set(app.ui.lbl_conn,    'String', 'Desconectado', 'ForegroundColor', C_RED);
        set([app.ui.btn_trigger app.ui.btn_save], 'Enable', 'off');
        set(app.ui.lbl_status,  'String', 'Estado: Desconectado', 'ForegroundColor', [0 0 0]);
        set(app.ui.lbl_samples, 'String', 'Muestras: —');
        guidata(fig, app);
    end

% ── Recepción serial (callback por línea) ───────────────────────────────
    function serialRxCb(src, ~)
        app = guidata(fig);
        try
            line = strtrim(char(readline(src)));
        catch
            return;
        end
        if isempty(line); return; end

        % ── Marcador inicio de trama ──
        if strcmpi(line, 'S') || strcmpi(line, 'START')
            app.rx_count  = 0;
            app.rx_floats = zeros(1, app.LONGITUD);
            app.in_frame  = true;
            app.receiving = true;
            set(app.ui.lbl_status, 'String', 'Estado: RECIBIENDO...', ...
                'ForegroundColor', C_ORG);
            set(app.ui.lbl_samples, 'String', sprintf('Muestras: 0 / %d', app.LONGITUD));
            guidata(fig, app);
            return;
        end

        % ── Marcador fin de trama ──
        if strcmpi(line, 'E') || strcmpi(line, 'END')
            app.in_frame  = false;
            app.receiving = false;
            if app.rx_count > 0
                app = closeFrame(app);
            end
            guidata(fig, app);
            return;
        end

        % ── Valor numérico ──
        val = str2double(line);
        if isnan(val); guidata(fig, app); return; end

        % Auto-inicio de trama sin marcadores
        if ~app.in_frame
            app.rx_count  = 0;
            app.rx_floats = zeros(1, app.LONGITUD);
            app.in_frame  = true;
            app.receiving = true;
            set(app.ui.lbl_status, 'String', 'Estado: RECIBIENDO (auto)...', ...
                'ForegroundColor', C_ORG);
        end

        if app.rx_count < app.LONGITUD
            app.rx_count = app.rx_count + 1;
            app.rx_floats(app.rx_count) = val;

            if mod(app.rx_count, 50) == 0
                pct = round(100 * app.rx_count / app.LONGITUD);
                set(app.ui.lbl_samples, 'String', ...
                    sprintf('Muestras: %d / %d  (%d%%)', ...
                    app.rx_count, app.LONGITUD, pct));
                drawnow limitrate;
            end

            % Cierre automático al completar LONGITUD muestras
            if app.rx_count >= app.LONGITUD
                app.in_frame  = false;
                app.receiving = false;
                app = closeFrame(app);
            end
        end

        guidata(fig, app);
    end

% ── Procesar trama completa ──────────────────────────────────────────────
    function app = closeFrame(app)
        n      = app.rx_count;
        fs_val = str2double(get(app.ui.edit_fs, 'String'));
        if isnan(fs_val) || fs_val <= 0; fs_val = 10000; end

        app.cur_signal = app.rx_floats(1:n)';        % columna
        app.cur_time   = ((0:n-1) / fs_val * 1000)'; % ms, columna

        doUpdatePlot(app);
        doUpdateStats(app);

        set(app.ui.lbl_status, 'String', ...
            sprintf('Estado: Listo — %d muestras @ %.0f Hz', n, fs_val), ...
            'ForegroundColor', C_GRN);
        set(app.ui.lbl_samples, 'String', ...
            sprintf('Muestras: %d / %d', n, app.LONGITUD));
        set(app.ui.btn_save, 'Enable', 'on');
    end

% ── Disparar adquisición ─────────────────────────────────────────────────
    function onTrigger(~, ~)
        app = guidata(fig);
        if ~app.connected || isempty(app.serial_obj); return; end
        if app.receiving; return; end

        app.rx_count  = 0;
        app.in_frame  = false;
        app.receiving = false;

        try
            write(app.serial_obj, uint8('T'), 'uint8');
        catch err
            % Si el PSoC no implementa recepción de T, continúa en modo escucha
            fprintf('Advertencia al enviar trigger: %s\n', err.message);
        end

        set(app.ui.lbl_status, 'String', ...
            'Estado: Trigger enviado — Esperando trama...', ...
            'ForegroundColor', [0.45 0 0.65]);
        set(app.ui.lbl_samples, 'String', sprintf('Muestras: 0 / %d', app.LONGITUD));
        guidata(fig, app);
    end

% ── Guardar señal en .mat ────────────────────────────────────────────────
    function onSave(~, ~)
        app = guidata(fig);
        if isempty(app.cur_signal)
            warndlg('No hay señal disponible para guardar.', 'Aviso'); return;
        end

        % Leer pilote seleccionado en el popup
        [nombre_pilote, stk] = getSelectedPilote(app);
        if isempty(nombre_pilote)
            warndlg('Registrá al menos un pilote antes de guardar.', 'Aviso'); return;
        end

        prof   = str2double(get(app.ui.edit_prof,    'String'));
        fs_val = str2double(get(app.ui.edit_fs,      'String'));
        mfile  = strtrim(get(app.ui.edit_matfile,  'String'));

        if isnan(prof);    prof   = 0;                end
        if isnan(fs_val);  fs_val = 10000;            end
        if isempty(mfile); mfile  = 'datos_pozos.mat'; end

        % Construir registro — siempre append, la fecha diferencia las mediciones
        nuevo.pilote      = nombre_pilote;
        nuevo.profundidad = prof;
        nuevo.stickup     = stk;
        nuevo.fs          = fs_val;
        nuevo.senal       = app.cur_signal;
        nuevo.tiempo      = app.cur_time;
        nuevo.fecha       = char(datetime('now', 'Format', 'yyyy-MM-dd HH:mm:ss'));
        nuevo.n_muestras  = length(app.cur_signal);

        % Cargar datos y pilotes existentes
        datos   = loadDatos(mfile);
        pilotes = loadPilotes(mfile); %#ok<NASGU>

        if isempty(datos)
            datos = nuevo;
        else
            datos(end+1) = nuevo;
        end

        try
            save(mfile, 'datos', 'pilotes');
            app.mat_file = mfile;
            guidata(fig, app);
            set(app.ui.lbl_status, 'String', ...
                sprintf('Estado: GUARDADO — %s @ %.2f m  [%s]', ...
                nombre_pilote, prof, nuevo.fecha), ...
                'ForegroundColor', C_BLU);
            refreshRecordsList();
        catch err
            errordlg(['Error al guardar: ' err.message], 'Error');
        end
    end

% ── Examinar archivo .mat ────────────────────────────────────────────────
    function onBrowse(~, ~)
        [fn, fp] = uiputfile('*.mat', 'Seleccionar archivo de datos', 'datos_pozos.mat');
        if isequal(fn, 0); return; end
        app = guidata(fig);
        fp_full = fullfile(fp, fn);
        set(app.ui.edit_matfile, 'String', fp_full);
        app.mat_file = fp_full;
        guidata(fig, app);
        refreshPilotesList();
        refreshRecordsList();
    end

% ── Zoom ─────────────────────────────────────────────────────────────────
    function onZoomApply(~, ~)
        app = guidata(fig);
        t0  = str2double(get(app.ui.edit_t0, 'String'));
        t1  = str2double(get(app.ui.edit_t1, 'String'));
        if isnan(t0) || isnan(t1) || t0 >= t1
            warndlg('Ingrese un rango válido: Desde < Hasta.', 'Zoom'); return;
        end
        xlim(app.ui.ax, [t0 t1]);
    end

    function onZoomReset(~, ~)
        app = guidata(fig);
        xlim(app.ui.ax, 'auto');
        if ~isempty(app.cur_time)
            set(app.ui.edit_t0, 'String', '0');
            set(app.ui.edit_t1, 'String', sprintf('%.3f', app.cur_time(end)));
        end
    end

% ── Cargar registro seleccionado ─────────────────────────────────────────
    function onLoadRecord(~, ~)
        app   = guidata(fig);
        mfile = strtrim(get(app.ui.edit_matfile, 'String'));
        if exist(mfile, 'file') ~= 2
            warndlg('Archivo .mat no encontrado.', 'Aviso'); return;
        end
        sel   = get(app.ui.list_rec, 'Value');
        items = get(app.ui.list_rec, 'String');
        if isempty(items) || sel > length(items); return; end

        try
            datos = loadDatos(mfile);
            if sel > length(datos); return; end
            rec = datos(sel);

            app.cur_signal = rec.senal(:);
            app.cur_time   = rec.tiempo(:);

            % Seleccionar el pilote en el dropdown si existe
            nombres = get(app.ui.pop_pilote, 'String');
            match   = find(strcmpi(nombres, rec.pilote), 1);
            if ~isempty(match)
                set(app.ui.pop_pilote, 'Value', match);
                onPiloteChanged([], []);
            end
            set(app.ui.edit_prof, 'String', num2str(rec.profundidad));
            set(app.ui.edit_fs,   'String', num2str(rec.fs));

            doUpdatePlot(app);
            doUpdateStats(app);
            set(app.ui.lbl_status, 'String', ...
                sprintf('Cargado: %s @ %.2f m  —  %s', ...
                rec.pilote, rec.profundidad, rec.fecha), ...
                'ForegroundColor', C_BLU);
            set(app.ui.btn_save, 'Enable', 'on');
            guidata(fig, app);
        catch err
            errordlg(['Error al cargar: ' err.message], 'Error');
        end
    end

% ── Cierre de figura ─────────────────────────────────────────────────────
    function onClose(~, ~)
        app = guidata(fig);
        try
            if ~isempty(app.serial_obj) && isvalid(app.serial_obj)
                configureCallback(app.serial_obj, 'off');
                delete(app.serial_obj);
            end
        catch
        end
        delete(fig);
    end

% #########################################################################
% HELPERS
% #########################################################################

    function doUpdatePlot(app)
        if isempty(app.cur_signal); return; end
        set(app.ui.hline, 'XData', app.cur_time, 'YData', app.cur_signal);

        % Aplicar ventana de zoom si el rango es válido
        t0 = str2double(get(app.ui.edit_t0, 'String'));
        t1 = str2double(get(app.ui.edit_t1, 'String'));
        if ~isnan(t0) && ~isnan(t1) && t0 < t1 && t1 <= app.cur_time(end) * 1.001
            xlim(app.ui.ax, [t0 t1]);
        else
            xlim(app.ui.ax, 'auto');
        end
        ylim(app.ui.ax, 'auto');

        [pilote, stk] = getSelectedPilote(app);
        prof          = str2double(get(app.ui.edit_prof, 'String'));
        if isnan(prof); prof = 0; end

        title(app.ui.ax, ...
            sprintf('Pilote: %s  |  Prof: %.2f m  |  Stickup: %.2f m  |  N = %d pts', ...
            pilote, prof, stk, length(app.cur_signal)), 'FontSize', 10);
        drawnow;
    end

    function doUpdateStats(app)
        if isempty(app.cur_signal); return; end
        s = app.cur_signal;
        set(app.ui.lbl_max,  'String', sprintf('%.4f', max(s)));
        set(app.ui.lbl_min,  'String', sprintf('%.4f', min(s)));
        set(app.ui.lbl_mean, 'String', sprintf('%.4f', mean(s)));
        set(app.ui.lbl_rms,  'String', sprintf('%.4f', rms(s)));
        set(app.ui.lbl_dur,  'String', sprintf('%.3f',  app.cur_time(end)));
    end

    function refreshRecordsList()
        app   = guidata(fig);
        mfile = strtrim(get(app.ui.edit_matfile, 'String'));
        items = {};
        n     = 0;
        if exist(mfile, 'file') == 2
            try
                datos = loadDatos(mfile);
                n = length(datos);
                for idx = 1:n
                    d = datos(idx);
                    items{end+1} = sprintf('[%d]  %-10s  %6.2f m  stk %5.2f m  %s', ...
                        idx, d.pilote, d.profundidad, d.stickup, d.fecha); %#ok<AGROW>
                end
            catch
                items = {'[Error al leer archivo]'};
            end
        else
            items = {'(archivo aún no creado)'};
        end
        cur_val = get(app.ui.list_rec, 'Value');
        set(app.ui.list_rec, 'String', items, ...
            'Value', max(1, min(cur_val, max(1, n))));
        set(app.ui.lbl_nrec, 'String', sprintf('Registros: %d', n));
    end

    function datos = loadDatos(mfile)
        % Carga el struct array 'datos' desde el .mat; devuelve [] si falla
        datos = [];
        if exist(mfile, 'file') ~= 2; return; end
        try
            tmp   = load(mfile, 'datos');
            datos = tmp.datos;
        catch
        end
    end

% ── Gestión de pilotes ───────────────────────────────────────────────────

    function onNewPilote(~, ~)
        % Abre la ventana modal de gestión de pilotes
        app   = guidata(fig);
        mfile = strtrim(get(app.ui.edit_matfile, 'String'));
        if isempty(mfile); mfile = 'datos_pozos.mat'; end

        % ── Figura modal ────────────────────────────────────────────────
        dlg = figure( ...
            'Name',        'Gestión de Pilotes', ...
            'NumberTitle', 'off', ...
            'Position',    [200 200 420 400], ...
            'Resize',      'off', ...
            'Color',       C_BG, ...
            'WindowStyle', 'modal', ...
            'CloseRequestFcn', @dlgClose);

        % ── Lista de pilotes existentes ──────────────────────────────────
        uicontrol(dlg, 'Style', 'text', ...
            'String', 'Pilotes registrados:', ...
            'Units', 'pixels', 'Position', [10 360 200 22], ...
            'HorizontalAlignment', 'left', 'BackgroundColor', C_BG, ...
            'FontWeight', 'bold');

        dlg_list = uicontrol(dlg, 'Style', 'listbox', ...
            'Units', 'pixels', 'Position', [10 230 400 128], ...
            'FontSize', 9, 'BackgroundColor', [1 1 1]);

        % ── Separador y título sección alta ─────────────────────────────
        uicontrol(dlg, 'Style', 'text', ...
            'String', '─── Nuevo pilote ───────────────────────────', ...
            'Units', 'pixels', 'Position', [10 200 400 20], ...
            'HorizontalAlignment', 'left', 'BackgroundColor', C_BG, ...
            'ForegroundColor', [0.4 0.4 0.4]);

        % Nombre
        uicontrol(dlg, 'Style', 'text', 'String', 'Nombre / ID:', ...
            'Units', 'pixels', 'Position', [10 168 110 22], ...
            'HorizontalAlignment', 'left', 'BackgroundColor', C_BG);
        dlg_nombre = uicontrol(dlg, 'Style', 'edit', 'String', '', ...
            'Units', 'pixels', 'Position', [125 168 275 24], ...
            'BackgroundColor', [1 1 1]);

        % Stickup
        uicontrol(dlg, 'Style', 'text', 'String', 'Stickup (m):', ...
            'Units', 'pixels', 'Position', [10 132 110 22], ...
            'HorizontalAlignment', 'left', 'BackgroundColor', C_BG);
        dlg_stickup = uicontrol(dlg, 'Style', 'edit', 'String', '0.500', ...
            'Units', 'pixels', 'Position', [125 132 120 24], ...
            'BackgroundColor', [1 1 1]);

        % ── Botones ──────────────────────────────────────────────────────
        uicontrol(dlg, 'Style', 'pushbutton', 'String', 'AGREGAR', ...
            'Units', 'pixels', 'Position', [10 88 120 32], ...
            'FontWeight', 'bold', 'BackgroundColor', C_GRN, ...
            'ForegroundColor', 'white', 'Callback', @dlgAgregar);

        uicontrol(dlg, 'Style', 'pushbutton', 'String', 'ELIMINAR SELECCIONADO', ...
            'Units', 'pixels', 'Position', [140 88 270 32], ...
            'FontWeight', 'bold', 'BackgroundColor', C_RED, ...
            'ForegroundColor', 'white', 'Callback', @dlgEliminar);

        uicontrol(dlg, 'Style', 'pushbutton', 'String', 'CERRAR', ...
            'Units', 'pixels', 'Position', [10 44 400 32], ...
            'FontWeight', 'bold', 'BackgroundColor', [0.4 0.4 0.4], ...
            'ForegroundColor', 'white', 'Callback', @dlgClose);

        uicontrol(dlg, 'Style', 'text', 'String', '', ...
            'Tag', 'dlg_status', ...
            'Units', 'pixels', 'Position', [10 12 400 26], ...
            'HorizontalAlignment', 'left', 'BackgroundColor', C_BG, ...
            'ForegroundColor', C_BLU, 'FontWeight', 'bold');

        dlgRefreshList();   % poblar lista inicial
        uiwait(dlg);        % bloquear hasta que se cierre

        % Al volver, refrescar dropdown principal
        refreshPilotesList();
        onPiloteChanged([], []);

        % ── Callbacks internos del diálogo ───────────────────────────────
        function dlgRefreshList()
            pilotes = loadPilotes(mfile);
            if isempty(pilotes)
                set(dlg_list, 'String', {'(no hay pilotes registrados)'}, 'Value', 1);
            else
                items = cell(1, length(pilotes));
                for jj = 1:length(pilotes)
                    items{jj} = sprintf('%-14s  stk: %.3f m    [%s]', ...
                        pilotes(jj).nombre, pilotes(jj).stickup, pilotes(jj).fecha_reg);
                end
                cur = min(get(dlg_list, 'Value'), length(items));
                set(dlg_list, 'String', items, 'Value', max(1, cur));
            end
        end

        function dlgAgregar(~, ~)
            nombre = strtrim(get(dlg_nombre, 'String'));
            stk    = str2double(get(dlg_stickup, 'String'));
            lbl    = findobj(dlg, 'Tag', 'dlg_status');

            if isempty(nombre)
                set(lbl, 'String', 'Error: el nombre no puede estar vacío.', ...
                    'ForegroundColor', C_RED); return;
            end
            if isnan(stk) || stk < 0
                set(lbl, 'String', 'Error: stickup debe ser un número ≥ 0.', ...
                    'ForegroundColor', C_RED); return;
            end

            pilotes = loadPilotes(mfile);
            for jj = 1:length(pilotes)
                if strcmpi(pilotes(jj).nombre, nombre)
                    set(lbl, 'String', ...
                        sprintf('Error: "%s" ya existe.', nombre), ...
                        'ForegroundColor', C_RED); return;
                end
            end

            nuevo_p.nombre    = nombre;
            nuevo_p.stickup   = stk;
            nuevo_p.fecha_reg = char(datetime('now', 'Format', 'yyyy-MM-dd HH:mm:ss'));

            if isempty(pilotes); pilotes = nuevo_p;
            else;                pilotes(end+1) = nuevo_p;
            end

            datos = loadDatos(mfile); %#ok<NASGU>
            try
                save(mfile, 'datos', 'pilotes');
                set(dlg_nombre,  'String', '');
                set(dlg_stickup, 'String', '0.500');
                set(lbl, 'String', sprintf('Pilote "%s" agregado.', nombre), ...
                    'ForegroundColor', C_GRN);
                dlgRefreshList();
            catch err
                set(lbl, 'String', ['Error al guardar: ' err.message], ...
                    'ForegroundColor', C_RED);
            end
        end

        function dlgEliminar(~, ~)
            pilotes = loadPilotes(mfile);
            lbl     = findobj(dlg, 'Tag', 'dlg_status');
            if isempty(pilotes)
                set(lbl, 'String', 'No hay pilotes para eliminar.', ...
                    'ForegroundColor', C_RED); return;
            end

            sel = get(dlg_list, 'Value');
            if sel < 1 || sel > length(pilotes); return; end

            nombre_del = pilotes(sel).nombre;
            confirm = questdlg( ...
                sprintf('¿Eliminar el pilote "%s"?\nEsta acción no elimina las mediciones asociadas.', nombre_del), ...
                'Confirmar', 'Eliminar', 'Cancelar', 'Cancelar');
            if ~strcmp(confirm, 'Eliminar'); return; end

            pilotes(sel) = [];
            datos = loadDatos(mfile); %#ok<NASGU>
            try
                save(mfile, 'datos', 'pilotes');
                set(lbl, 'String', sprintf('Pilote "%s" eliminado.', nombre_del), ...
                    'ForegroundColor', C_ORG);
                dlgRefreshList();
            catch err
                set(lbl, 'String', ['Error al guardar: ' err.message], ...
                    'ForegroundColor', C_RED);
            end
        end

        function dlgClose(~, ~)
            uiresume(dlg);
            delete(dlg);
        end
    end

    function onPiloteChanged(~, ~)
        % Actualiza el label de stickup cuando cambia la selección del dropdown
        app     = guidata(fig);
        [~, stk] = getSelectedPilote(app);
        if isnan(stk)
            set(app.ui.lbl_stickup_val, 'String', '—');
        else
            set(app.ui.lbl_stickup_val, 'String', sprintf('%.3f', stk));
        end
    end

    function refreshPilotesList()
        app   = guidata(fig);
        mfile = strtrim(get(app.ui.edit_matfile, 'String'));

        pilotes = loadPilotes(mfile);
        app.pilotes = pilotes;
        guidata(fig, app);

        if isempty(pilotes)
            set(app.ui.pop_pilote, 'String', {'(sin pilotes)'}, 'Value', 1);
            set(app.ui.lbl_stickup_val, 'String', '—');
        else
            nombres = {pilotes.nombre};
            cur_val = get(app.ui.pop_pilote, 'Value');
            set(app.ui.pop_pilote, 'String', nombres, ...
                'Value', min(cur_val, length(nombres)));
            onPiloteChanged([], []);
        end
    end

    function pilotes = loadPilotes(mfile)
        pilotes = [];
        if exist(mfile, 'file') ~= 2; return; end
        try
            tmp = load(mfile, 'pilotes');
            if isfield(tmp, 'pilotes')
                pilotes = tmp.pilotes;
            end
        catch
        end
    end

    function [nombre, stk] = getSelectedPilote(app)
        % Devuelve el nombre y stickup del pilote seleccionado en el popup
        nombre = '';
        stk    = NaN;
        if isempty(app.pilotes); return; end
        idx = get(app.ui.pop_pilote, 'Value');
        if idx < 1 || idx > length(app.pilotes); return; end
        nombre = app.pilotes(idx).nombre;
        stk    = app.pilotes(idx).stickup;
    end

% ── Factories de controles (evitan repetición) ───────────────────────────
    function h = mkpanel(parent, ttl, pos)
        h = uipanel(parent, 'Title', ttl, ...
            'Units', 'normalized', 'Position', pos, ...
            'FontWeight', 'bold', 'FontSize', 9.5, ...
            'BackgroundColor', C_BG);
    end

    function h = mktext(parent, str, pos)
        h = uicontrol(parent, 'Style', 'text', 'String', str, ...
            'Units', 'normalized', 'Position', pos, ...
            'HorizontalAlignment', 'left', 'BackgroundColor', C_BG);
    end

    function h = mkvaltext(parent, str, pos)
        h = uicontrol(parent, 'Style', 'text', 'String', str, ...
            'Units', 'normalized', 'Position', pos, ...
            'HorizontalAlignment', 'right', 'FontWeight', 'bold', ...
            'BackgroundColor', C_BG);
    end

    function h = mkedit(parent, str, pos)
        h = uicontrol(parent, 'Style', 'edit', 'String', str, ...
            'Units', 'normalized', 'Position', pos, ...
            'BackgroundColor', C_WHT);
    end

    function h = mkbtn(parent, str, pos, bg, cb)
        h = uicontrol(parent, 'Style', 'pushbutton', 'String', str, ...
            'Units', 'normalized', 'Position', pos, ...
            'FontWeight', 'bold', 'BackgroundColor', bg, ...
            'ForegroundColor', 'white', 'Callback', cb);
        % Botones con fondo claro → texto negro
        if mean(bg) > 0.7
            set(h, 'ForegroundColor', [0 0 0]);
        end
    end

end % interfaz_transductor
