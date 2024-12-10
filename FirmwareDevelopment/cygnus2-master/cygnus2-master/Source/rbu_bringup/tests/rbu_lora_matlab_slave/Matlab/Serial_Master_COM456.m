% Load following mbed images:
%
% COM4 = SX1276_Matlab_SlaveNUCLEO_L152RE.bin
% COM5 = SX1276_Matlab_SlaveNUCLEO_L152RE.bin
% COM6 = SX1276_Matlab_SlaveNUCLEO_L152RE.bin

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
set(obj1, 'Timeout', 10);

set(obj2, 'ReadAsyncMode', 'continuous');
set(obj2, 'FlowControl', 'none');
set(obj2, 'BaudRate', 9600);
set(obj2, 'DataBits', 8);
set(obj2, 'Parity', 'none');
set(obj2, 'StopBit', 1);
set(obj2, 'Timeout', 10);

set(obj3, 'ReadAsyncMode', 'continuous');
set(obj3, 'FlowControl', 'none');
set(obj3, 'BaudRate', 9600);
set(obj3, 'DataBits', 8);
set(obj3, 'Parity', 'none');
set(obj3, 'StopBit', 1);
set(obj3, 'Timeout', 10);

% Configure to execute the callback function instrcallback when a bytes-available event
% or an output-empty event occurs. Because instrcallback requires the serial port object
% and event information to be passed as input arguments, the callback function is specified
% as a function handle.
obj1.BytesAvailableFcnMode = 'terminator';
obj1.Terminator = 'CR/LF';
obj1.BytesAvailableFcn = @instrcallback;
obj1.OutputEmptyFcn = @instrcallback;
obj2.BytesAvailableFcnMode = 'terminator';
obj2.Terminator = 'CR/LF';
obj2.BytesAvailableFcn = @instrcallback;
obj2.OutputEmptyFcn = @instrcallback;
obj3.BytesAvailableFcnMode = 'terminator';
obj3.Terminator = 'CR/LF';
obj3.BytesAvailableFcn = @instrcallback;
obj3.OutputEmptyFcn = @instrcallback;

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
set(findobj(msgbox1,'style','pushbutton'),'Visible','off');
set(findobj(msgbox2,'style','pushbutton'),'Visible','off');
set(findobj(msgbox3,'style','pushbutton'),'Visible','off');

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

flushinput(obj1);
flushinput(obj2);
flushinput(obj3);
flushoutput(obj1);
flushoutput(obj2);
flushoutput(obj3);

dcount1 = 0;
dcount2 = 0;
dcount3 = 0;
help1 = 1;
help2 = 1;
help3 = 1;
receive = 0;
transmit1 = 0;
transmit2 = 0;

while 1    % loop forever
%repeats = 100;
%for i = 1:repeats    % loop 'repeats' times
    % Send receive command (R) if data response (D), error (E), timeout (T), or comment (#)
    if (obj1.BytesAvailable)
        [A, count1, msg1] = fscanf(obj1);
        flushinput(obj1);
        obj1op = sscanf(A, '%c', 1);
        if ((obj1op == 'D') | (obj1op == 'E') | (obj1op == 'T') | (obj1op == '#'))
            if (obj1op == 'D')
                dcount1 = dcount1 + 1;
            end
            receive = 1;
        end
        set(findobj(msgbox1,'Tag','MessageBox'),'String',A);
    else
        if (help1 == 1)
            fprintf(obj1,'%s\r','H');
            help1 = 0;
        end
        if (receive == 1)
            fprintf(obj1,'%s\r','R');
            receive = 0;
        end
    end

    if (obj2.BytesAvailable)
        [B, count2, msg2] = fscanf(obj2);
        flushinput(obj2);
        obj2op = sscanf(B, '%c', 1);
        if ((obj2op == 'D') | (obj2op == 'E') | (obj2op == 'T') | (obj2op == '#'))
            if (obj2op == 'D')
                dcount2 = dcount2 + 1;
            end
            transmit1 = 1;
        end
        set(findobj(msgbox2,'Tag','MessageBox'),'String',B);
    else
        if (help2 == 1)
            fprintf(obj2,'%s\r','H');
            help2 = 0;
        end
        if (transmit1 == 1)
            fprintf(obj2,'%s\r','T ONE1');
            transmit1 = 0;
        end
    end
    
    if (obj3.BytesAvailable)
        [C, count3, msg3] = fscanf(obj3);
        flushinput(obj3);
        obj3op = sscanf(C, '%c', 1);
        if ((obj3op == 'D') | (obj3op == 'E') | (obj3op == 'T') | (obj3op == '#'))
            if (obj3op == 'D')
                dcount3 = dcount3 + 1;
            end
            transmit2 = 1;
        end
        set(findobj(msgbox3,'Tag','MessageBox'),'String',C);
    else
        if (help3 == 1)
            fprintf(obj3,'%s\r','H');
            help3 = 0;
        end
        if (transmit2 == 1)
            fprintf(obj3,'%s\r','T TWO2');
            transmit2 = 0;
        end
    end

    drawnow update
end

% Free the serial port objects
fclose(obj1);
fclose(obj2);
fclose(obj3);
delete(obj1);
delete(obj2);
delete(obj3);
clear obj1;
clear obj2;
clear obj3;
delete(msgbox1);
delete(msgbox2);
delete(msgbox3);
