local message = 3

function onTime(time)

	doSetGameState(GAMESTATE_CLOSING)
	alertShudown()
	
	return true
end

function alertShudown()
	if(message <= 0) then
		doSetGameState(GAMESTATE_SHUTDOWN)
		return false
	end

	if(message == 1) then
		doBroadcastMessage("The game will be shutdown in less than one minute for global server save, please log out now!")
		addEvent(alertShudown, 60000)
		message = message - 1
	elseif(message == 2) then
		doBroadcastMessage("The game will be shutdown in 3 minutes for global server save, please log out.")
		addEvent(alertShudown, 60000 * 2)
		message = message - 1
	else
		doBroadcastMessage("The game will be shutdown in 5 minutes for global server save. The game will come back again in 15 minutes.")
		addEvent(alertShudown, 60000 * 2)
		message = message - 1
	end
	
	return true
end