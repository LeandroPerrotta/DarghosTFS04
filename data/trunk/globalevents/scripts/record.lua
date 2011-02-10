function onRecord(current, old, cid)

	db.executeQuery("INSERT INTO `server_record` (`record`, `world_id`, `timestamp`) VALUES (" .. current .. ", " .. getConfigValue('worldId') .. ", " .. os.time() .. ");")
	
	if(darghos_spoof_players) then
		current = current + darghos_players_to_spoof
	end	
	
	addEvent(doBroadcastMessage, 150, "New record: " .. current .. " players are logged in.", MESSAGE_STATUS_DEFAULT)
end
