PLAYERS_MIN_LEVEL = 12
PLAYERS_MAX_LEVEL = 25

function spoofPlayers()

	db.executeQuery("UPDATE `players` SET `online` = '0', `is_spoof` = '0', `lastlogout` = '" .. os.time() .. "' WHERE `is_spoof` = '1';")
	spoofPlayersList = {}

	if(not darghos_spoof_players) then
		return true
	end

	local result = db.getResult("SELECT COUNT(*) as `rowscount` FROM `players` WHERE `group_id` <= '2' AND `level` >= '" .. PLAYERS_MIN_LEVEL .. "' AND `level` <= '" .. PLAYERS_MAX_LEVEL .. "' AND `online` = '0';")
	if(result:getID() == -1) then
		--print("[Spoofing] Players list not found.")
		return true
	end

	local rowscount = result:getDataInt("rowscount")
	result:free()	
	
	--print("[Spoofing] Player list found (" .. rowscount .. " players)")
	
	local i = 1
	
	while(i <= darghos_players_to_spoof) do
	
		local rand = math.random(1, rowscount)
		--print("[Spoofing] Checking player rand " .. rand .. ".")
	
		local result = db.getResult("SELECT `id`, `name`, `level`, `account_id` FROM `players` WHERE `group_id` <= '2' AND `level` >= '" .. PLAYERS_MIN_LEVEL .. "' AND `level` <= '" .. PLAYERS_MAX_LEVEL .. "' AND `online` = '0' LIMIT ".. rand ..", 1");
		if(result:getID() ~= -1) then
				
			local pid, pname, plevel, pacc = result:getDataInt("id"), result:getDataString("name"), result:getDataInt("level"), result:getDataInt("account_id")
			
			local accchk = db.getResult("SELECT COUNT(*) as `rowscount` FROM `players` WHERE `account_id` = '" .. pacc .. "' AND `online` = '1'")
		
			if(accchk:getID() ~= -1) then
				if(accchk:getDataInt("rowscount") == 0) then
				
					local player = {name = pname, level = plevel}
					table.insert(spoofPlayersList, player)
					
					--print("[Spoofing] Player " .. pname .. " with level " .. plevel .. " has been spoofed.")
					db.executeQuery("UPDATE `players` SET `online` = '1', `is_spoof` = '1', `lastlogin` = '" .. os.time() .. "' WHERE `id` = '" .. pid .. "';")
					i = i + 1
				else
					--print("[Spoofing] Others characters online on same acc, goto next...")
				end	
			else
				--print("[Spoofing] Acc check fail.")
			end
			
			
			accchk:free()
			result:free()
		else
			--print("[Spoofing] Random player not found.")
		end			
	end
	
	--print("Spoofed:" .. #spoofPlayersList)
end

function onThink(interval)
	spoofPlayers()
	return true
end