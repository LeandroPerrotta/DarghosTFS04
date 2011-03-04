-- Custom Modules do Darghos para o NPC System padrão do Jiddo

D_CustomNpcModules = {}

function D_CustomNpcModules.travelTrainingIsland(cid, message, keywords, parameters, node)
	local npcHandler = parameters.npcHandler
	if(npcHandler == nil) then
		print('[Warning - ' .. getCreatureName(getNpcId()) .. '] NpcSystem:', 'StdModule.travel - Call without any npcHandler instance.')
		return false
	end

	if(not npcHandler:isFocused(cid)) then
		return false
	end

	local storage, pzLocked = parameters.storageValue or (EMPTY_STORAGE + 1), parameters.allowLocked or false
	if(parameters.premium and not isPlayerPremiumCallback(cid)) then
		npcHandler:say('I can only allow premium players to travel with me.', cid)
	elseif(parameters.level ~= nil and getPlayerLevel(cid) < parameters.level) then
		npcHandler:say('You must reach level ' .. parameters.level .. ' before I can let you go there.', cid)
	elseif(parameters.storageId ~= nil and getPlayerStorageValue(cid, parameters.storageId) < storage) then
		npcHandler:say(parameters.storageInfo or 'You may not travel there!', cid)
	elseif(not pzLocked and isPlayerPzLocked(cid)) then
		npcHandler:say('Get out of there with this blood!', cid)
	elseif(not doPlayerRemoveMoney(cid, parameters.cost)) then
		npcHandler:say('You do not have enough money.', cid)
	else
		npcHandler:say('It was a pleasure doing business with you.', cid)
		npcHandler:releaseFocus(cid)
		
		if(parameters.entering ~= nil and parameters.entering) then
			setPlayerStorageValue(cid, sid.RECOVERING_STAMINA, 1)
		else
			setPlayerStorageValue(cid, sid.RECOVERING_STAMINA, STORAGE_NULL)
			setPlayerStorageValue(cid, sid.NEXT_STAMINA_UPDATE, STORAGE_NULL)
		end

		doTeleportThing(cid, parameters.destination, false)
		doSendMagicEffect(parameters.destination, CONST_ME_TELEPORT)
	end

	npcHandler:resetNpc()
	return true
end

function D_CustomNpcModules.pvpBless(cid, message, keywords, parameters, node)

	local npcHandler = parameters.npcHandler
	if(npcHandler == nil) then
		print('[Warning - ' .. getCreatureName(getNpcId()) .. '] NpcSystem:', 'StdModule.bless - Call without any npcHandler instance.')
		return false
	end

	if(not npcHandler:isFocused(cid)) then
		return false
	end

	local price = parameters.baseCost
	if(getPlayerLevel(cid) > parameters.startLevel) then
		price = (price + ((math.min(parameters.endLevel, getPlayerLevel(cid)) - parameters.startLevel) * parameters.levelCost))
	end

	if(getPlayerPVPBlessing(cid)) then
		npcHandler:say("Gods have already blessed you with this blessing!", cid)
	elseif(not doPlayerRemoveMoney(cid, price)) then
		npcHandler:say("You don't have enough money for blessing.", cid)
	else
		npcHandler:say("You have been protected your regular blessings with the twist of fate!", cid)
		doPlayerSetPVPBlessing(cid)
	end

	npcHandler:resetNpc()
	return true
end