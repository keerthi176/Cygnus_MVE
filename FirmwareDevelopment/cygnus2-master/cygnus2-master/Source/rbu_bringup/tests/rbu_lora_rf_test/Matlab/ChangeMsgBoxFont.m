function ChangeMsgBoxFont(msgHandle)
	fontName = 'FixedWidth';
	fontSize = 12;

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
end
