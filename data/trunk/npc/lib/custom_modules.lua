-- Custom Modules do Darghos para o NPC System padrão do Jiddo

D_CustomNpcModules = {}

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