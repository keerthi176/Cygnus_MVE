% Load following mbed images:
%
% COM4 = SX1276_RXNUCLEO_L152RE.bin
% COM5 = SX1276_TX1_MainNUCLEO_L152RE.bin
% COM6 = SX1276_TX2_InterfererNUCLEO_L152RE.bin

% Find serial port objects
obj1 = instrfind('Type', 'serial', 'Port', 'COM4', 'Tag', '');
obj2 = instrfind('Type', 'serial', 'Port', 'COM5', 'Tag', '');
obj3 = instrfind('Type', 'serial', 'Port', 'COM6', 'Tag', '');

% Close COM port before opening if already open
if (0 == isempty(obj1))
    fclose(obj1);
    delete(obj1);
end
obj1 = serial('COM4', 'BaudRate', 9600);
if (0 == isempty(obj2))
    fclose(obj2);
    delete(obj2);
end
obj2 = serial('COM5', 'BaudRate', 9600);
if (0 == isempty(obj3))
    fclose(obj3);
    delete(obj3);
end
obj3 = serial('COM6', 'BaudRate', 9600);

set(obj1, 'ReadAsyncMode', 'continuous');
set(obj1, 'FlowControl', 'none');
set(obj1, 'BaudRate', 9600);
set(obj1, 'DataBits', 8);
set(obj1, 'Parity', 'none');
set(obj1, 'StopBit', 1);
set(obj1, 'Timeout', 1);

set(obj2, 'ReadAsyncMode', 'continuous');
set(obj2, 'FlowControl', 'none');
set(obj2, 'BaudRate', 9600);
set(obj2, 'DataBits', 8);
set(obj2, 'Parity', 'none');
set(obj2, 'StopBit', 1);
set(obj2, 'Timeout', 1);

set(obj3, 'ReadAsyncMode', 'continuous');
set(obj3, 'FlowControl', 'none');
set(obj3, 'BaudRate', 9600);
set(obj3, 'DataBits', 8);
set(obj3, 'Parity', 'none');
set(obj3, 'StopBit', 1);
set(obj3, 'Timeout', 1);

% Connect to instrument objects
fopen(obj1);
fopen(obj2);
fopen(obj3);

A='........................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................';
B=A;
C=A;

msgbox1 = msgbox(A, 'COM4');
msgbox2 = msgbox(B, 'COM5');
msgbox3 = msgbox(C, 'COM6');
set(findobj(msgbox1,'style','pushbutton'),'Visible','off')
set(findobj(msgbox2,'style','pushbutton'),'Visible','off')
set(findobj(msgbox3,'style','pushbutton'),'Visible','off')

fontName = 'FixedWidth';
fontSize = 12;
msgHandle = msgbox1;
% get handles to the UIControls ([OK] PushButton) and Text
kids0 = findobj( msgHandle, 'Type', 'UIControl' );
kids1 = findobj( msgHandle, 'Type', 'Text' );
% change the font and fontsize
extent0 = get( kids1, 'Extent' ); % text extent in old font
set( [kids0, kids1], 'FontName', fontName, 'FontSize', fontSize );
extent1 = get( kids1, 'Extent' ); % text extent in new font
% need to resize the msgbox object to accommodate new FontName 
% and FontSize
delta = extent1 - extent0; % change in extent
pos = get( msgHandle, 'Position' ); % msgbox current position
pos = pos + delta; % change size of msgbox
set( msgHandle, 'Position', pos ); % set new position

fontName = 'FixedWidth';
fontSize = 12;
msgHandle = msgbox2;
% get handles to the UIControls ([OK] PushButton) and Text
kids0 = findobj( msgHandle, 'Type', 'UIControl' );
kids1 = findobj( msgHandle, 'Type', 'Text' );
% change the font and fontsize
extent0 = get( kids1, 'Extent' ); % text extent in old font
set( [kids0, kids1], 'FontName', fontName, 'FontSize', fontSize );
extent1 = get( kids1, 'Extent' ); % text extent in new font
% need to resize the msgbox object to accommodate new FontName 
% and FontSize
delta = extent1 - extent0; % change in extent
pos = get( msgHandle, 'Position' ); % msgbox current position
pos = pos + delta; % change size of msgbox
set( msgHandle, 'Position', pos ); % set new position

fontName = 'FixedWidth';
fontSize = 12;
msgHandle = msgbox3;
% get handles to the UIControls ([OK] PushButton) and Text
kids0 = findobj( msgHandle, 'Type', 'UIControl' );
kids1 = findobj( msgHandle, 'Type', 'Text' );
% change the font and fontsize
extent0 = get( kids1, 'Extent' ); % text extent in old font
set( [kids0, kids1], 'FontName', fontName, 'FontSize', fontSize );
extent1 = get( kids1, 'Extent' ); % text extent in new font
% need to resize the msgbox object to accommodate new FontName 
% and FontSize
delta = extent1 - extent0; % change in extent
pos = get( msgHandle, 'Position' ); % msgbox current position
pos = pos + delta; % change size of msgbox
set( msgHandle, 'Position', pos ); % set new position

while 1    % loop forever
%repeats = 100;
%for i = 1:repeats    % loop 'repeats' times
    [A, count1, msg1] = fscanf(obj1)
    [B, count2, msg2] = fscanf(obj2)
    [C, count3, msg3] = fscanf(obj3)
    set(findobj(msgbox1,'Tag','MessageBox'),'String',A)
    set(findobj(msgbox2,'Tag','MessageBox'),'String',B)
    set(findobj(msgbox3,'Tag','MessageBox'),'String',C)
    drawnow update
end

% Free the serial port objects
fclose(obj1);
fclose(obj2);
fclose(obj3);
