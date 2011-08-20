function GUI_ErrorMonitors(varargin)
%GUI_ERRORMONITORS to define the layout of the error monitors page
%GUI_ErrorMonitors(varargin)
%
% varargin : variable length input argument list
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%                          OpenFresco Express                          %%
%%    GUI for the Open Framework for Experimental Setup and Control     %%
%%                                                                      %%
%%   (C) Copyright 2011, The Pacific Earthquake Engineering Research    %%
%%            Center (PEER) & MTS Systems Corporation (MTS)             %%
%%                         All Rights Reserved.                         %%
%%                                                                      %%
%%     Commercial use of this program without express permission of     %%
%%                 PEER and MTS is strictly prohibited.                 %%
%%     See Help -> OpenFresco Express Disclaimer for information on     %%
%%   usage and redistribution, and for a DISCLAIMER OF ALL WARRANTIES.  %%
%%                                                                      %%
%%   Developed by:                                                      %%
%%     Andreas Schellenberg (andreas.schellenberg@gmail.com)            %%
%%     Carl Misra (carl.misra@gmail.com)                                %%
%%     Stephen A. Mahin (mahin@berkeley.edu)                            %%
%%                                                                      %%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% $Revision$
% $Date$
% $URL$

% Check if window already exists
if ~isempty(findobj('Tag','ErrMon'))
    figure(findobj('Tag','ErrMon'));
    return
end

% Initialization tasks
handles = guidata(gcbf);

% find longest motion
tEnd = 0.0;
switch handles.GM.loadType
    case 'Ground Motions'
        for mo=1:length(handles.GM.dt)
            tEndi = handles.GM.scalet{mo}(end);
            if tEndi > tEnd
                tEnd = tEndi;
            end
        end
    case 'Initial Conditions'
        tEnd = handles.GM.rampTime + handles.GM.vibTime;
end

%%%%%%%%%%%%%%%
%Create figure%
%%%%%%%%%%%%%%%

%Main Figure
SS = get(0,'screensize');
f_ErrorMon = figure('Visible','on','Name','Error Monitors',...
    'NumberTitle','off',...
    'MenuBar','none',...
    'Tag','ErrMon',...
    'Color',[0.3 0.5 0.7],...
    'Position',[SS(3)*0.12,SS(4)*0.05,SS(3)*0.88,SS(4)*0.87]);

%Toolbar
File(1) = uimenu('Position',1,'Label','File');
uimenu(File(1),'Position',1,'Label','Save',...
    'Accelerator','S','Callback','filemenufcn(gcbf,''FileSaveAs'')');
uimenu(File(1),'Position',2,'Label','Print',...
    'Accelerator','P','Callback','printdlg(gcbf)');
Help(1) = uimenu('Position',2,'Label','Help');
uimenu(Help(1),'Position',1,'Label','About Error Plots','Callback','AxesHelp(''about error'')');
uimenu('Position',3,'Label','|');
StdMenu(1) = uimenu('Position',4,'Label','MATLAB Menu');
uimenu(StdMenu(1),'Position',1,'Label','Turn on', ...
   'Callback','set(gcf,''MenuBar'',''figure''); set(gcf,''Toolbar'',''figure'');');
uimenu(StdMenu(1),'Position',2,'Label','Turn off', ...
   'Callback','set(gcf,''MenuBar'',''none''); set(gcf,''Toolbar'',''none'');');


if strcmp(handles.Model.Type, '1 DOF')
    %1 DOF Case
    
    %Title
    uicontrol(f_ErrorMon,'Style','text',...
        'String','Error Monitors',...
        'FontSize',20,...
        'ForegroundColor',[1 1 1],...
        'BackgroundColor',[0.3 0.5 0.7],...
        'Units','normalized',...
        'Position',[0.3 0.87 0.4 0.1],...
        'FontName',handles.Store.font);
    
    %DOF 1 Figures
    a_ErrorMonitors_eX = axes('Parent',f_ErrorMon,...
        'Tag','EM1e',...
        'NextPlot','replacechildren',...
        'FontWeight','bold',...
        'ButtonDownFcn','AxesHelp(''e1'')',...
        'Position',[0.06 0.74 0.92 0.13],'Box','on',...
        'XLimMode','manual','Xlim',[0.0 tEnd]);
    grid('on');
    xlabel(a_ErrorMonitors_eX,'Time [sec]');
    ylabel(a_ErrorMonitors_eX,'Disp Error [L]');
    a_ErrorMonitors_efX = axes('Parent',f_ErrorMon,...
        'Tag','EM1ffte',...
        'NextPlot','replacechildren',...
        'FontWeight','bold',...
        'ButtonDownFcn','AxesHelp(''fft1'')',...
        'Position',[0.06 0.54 0.92 0.13],'Box','on');
    grid('on');
    xlabel(a_ErrorMonitors_efX,'Frequency [Hz]');
    ylabel(a_ErrorMonitors_efX,'Fourier Amp [L/sec]');
    a_ErrorMonitors_ddX = axes('Parent',f_ErrorMon,...
        'Tag','EM1MeasCmd',...
        'NextPlot','replacechildren',...
        'FontWeight','bold',...
        'ButtonDownFcn','AxesHelp(''dd1'')',...
        'Position',[0.06 0.07 0.42 0.41],'Box','on');
    grid('on');
    xlabel(a_ErrorMonitors_ddX,'Command Disp [L]');
    ylabel(a_ErrorMonitors_ddX,'Measured Disp [L]');
    a_ErrorMonitors_etX = axes('Parent',f_ErrorMon,...
        'Tag','EM1track',...
        'NextPlot','replacechildren',...
        'FontWeight','bold',...
        'ButtonDownFcn','AxesHelp(''TI1'')',...
        'Position',[0.56 0.07 0.42 0.41],'Box','on',...
        'XLimMode','manual','Xlim',[0.0 tEnd]);
    grid('on');
    xlabel(a_ErrorMonitors_etX,'Time [sec]');
    ylabel(a_ErrorMonitors_etX,'Tracking Indicator [L^2]');
    
else
    %2 DOF Case

    %Title
    uicontrol(f_ErrorMon,'Style','text',...
        'String','Error Monitors',...
        'FontSize',20,...
        'ForegroundColor',[1 1 1],...
        'BackgroundColor',[0.3 0.5 0.7],...
        'Units','normalized',...
        'Position',[0.3 0.92 0.4 0.05]);
    %DOF 1 Figures
    uicontrol(f_ErrorMon,'Style','text',...
        'Units','normalized',...
        'FontSize',15,...
        'ForegroundColor',[1 1 1],...
        'BackgroundColor',[0.3 0.5 0.7],...
        'String','DOF 1',...
        'Position',[0.26 0.85 0.1 0.05]);
    a_ErrorMonitors_eX = axes('Parent',f_ErrorMon,...
        'Tag','EM1e',...
        'NextPlot','replacechildren',...
        'FontWeight','bold',...
        'ButtonDownFcn','AxesHelp(''e1'')',...
        'Position',[0.06 0.72 0.43 0.13],'Box','on',...
        'XLimMode','manual','Xlim',[0.0 tEnd]);
    grid('on');
    xlabel(a_ErrorMonitors_eX,'Time [sec]');
    ylabel(a_ErrorMonitors_eX,'Disp Error [L]');    
    a_ErrorMonitors_efX = axes('Parent',f_ErrorMon,...
        'Tag','EM1ffte',...
        'NextPlot','replacechildren',...
        'FontWeight','bold',...
        'ButtonDownFcn','AxesHelp(''fft1'')',...
        'Position',[0.06 0.52 0.43 0.13],'Box','on');
    grid('on');
    xlabel(a_ErrorMonitors_efX,'Frequency [Hz]');
    ylabel(a_ErrorMonitors_efX,'Fourier Amp [L/sec]');
    a_ErrorMonitors_ddX = axes('Parent',f_ErrorMon,...
        'Tag','EM1MeasCmd',...
        'NextPlot','replacechildren',...
        'FontWeight','bold',...
        'ButtonDownFcn','AxesHelp(''dd1'')',...
        'Position',[0.06 0.08 0.19 0.38],'Box','on');
    grid('on');
    xlabel(a_ErrorMonitors_ddX,'Command Disp [L]');
    ylabel(a_ErrorMonitors_ddX,'Measured Disp [L]');
    a_ErrorMonitors_etX = axes('Parent',f_ErrorMon,...
        'Tag','EM1track',...
        'NextPlot','replacechildren',...
        'FontWeight','bold',...
        'ButtonDownFcn','AxesHelp(''TI1'')',...
        'Position',[0.3 0.08 0.19 0.38],'Box','on',...
        'XLimMode','manual','Xlim',[0.0 tEnd]);
    grid('on');
    xlabel(a_ErrorMonitors_etX,'Time [sec]');
    ylabel(a_ErrorMonitors_etX,'Tracking Indicator [L^2]');
    
    %DOF 2 Figures
    uicontrol(f_ErrorMon,'Style','text',...
        'Units','normalized',...
        'FontSize',15,...
        'ForegroundColor',[1 1 1],...
        'BackgroundColor',[0.3 0.5 0.7],...
        'String','DOF 2',...
        'Position',[0.72 0.85 0.1 0.05]);
    a_ErrorMonitors_eY = axes('Parent',f_ErrorMon,...
        'Tag','EM2e',...
        'NextPlot','replacechildren',...
        'FontWeight','bold',...
        'ButtonDownFcn','AxesHelp(''e2'')',...
        'Position',[0.55 0.72 0.43 0.13],'Box','on',...
        'XLimMode','manual','Xlim',[0.0 tEnd]);
    grid('on');
    xlabel(a_ErrorMonitors_eY,'Time [sec]');
    ylabel(a_ErrorMonitors_eY,'Disp Error [L]');
    a_ErrorMonitors_efY = axes('Parent',f_ErrorMon,...
        'Tag','EM2ffte',...
        'NextPlot','replacechildren',...
        'FontWeight','bold',...
        'ButtonDownFcn','AxesHelp(''fft2'')',...
        'Position',[0.55 0.52 0.43 0.13],'Box','on');
    grid('on');
    xlabel(a_ErrorMonitors_efY,'Frequency [Hz]');
    ylabel(a_ErrorMonitors_efY,'Fourier Amp [L/sec]');
    a_ErrorMonitors_ddY = axes('Parent',f_ErrorMon,...
        'Tag','EM2MeasCmd',...
        'NextPlot','replacechildren',...
        'FontWeight','bold',...
        'ButtonDownFcn','AxesHelp(''dd2'')',...
        'Position',[0.55 0.08 0.19 0.38],'Box','on');
    grid('on');
    xlabel(a_ErrorMonitors_ddY,'Command Disp [L]');
    ylabel(a_ErrorMonitors_ddY,'Measured Disp [L]');
    a_ErrorMonitors_etY = axes('Parent',f_ErrorMon,...
        'Tag','EM2track',...
        'NextPlot','replacechildren',...
        'FontWeight','bold',...
        'ButtonDownFcn','AxesHelp(''TI2'')',...
        'Position',[0.79 0.08 0.19 0.38],'Box','on',...
        'XLimMode','manual','Xlim',[0.0 tEnd]);
    grid('on');
    xlabel(a_ErrorMonitors_etY,'Time [sec]');
    ylabel(a_ErrorMonitors_etY,'Tracking Indicator [L^2]');
end
