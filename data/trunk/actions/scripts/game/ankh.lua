function onUse(cid, item, frompos, item2, topos)

	-- Divine Ankh Configurations
	local _vampireCorpses = {6006, 2956}
	local _ghoulCorpses = {5976, 3113}
	local checkPos = {x=2121, y=1883, z=7}
	local checkRange = 50

	local courseChurch = getPlayerStorageValue(cid, QUESTLOG.DIVINE_ANKH.COURSE_CHURCH)

	if(courseChurch == 2) then
	
		local pos = getCreaturePosition(cid)
		pos.z = 7
		
		local fromPos = {x=checkPos.x - checkRange, y=checkPos.y - checkRange, z=checkPos.z}		
		local toPos = {x=checkPos.x + checkRange, y=checkPos.y + checkRange, z=checkPos.z}	
		
		if(not isInRange(pos, fromPos, toPos)) then
			doPlayerSay(cid, "Eu estou muito longe da igreja! Devo retornar a suas proximidades!", TALKTYPE_ORANGE_1)
			return
		end
	
		local creatureToSummon = ""
	
		if(isInArray(_vampireCorpses, item2.itemid) == TRUE) then
		
			creatureToSummon = "Reborn Vampire"				
		elseif(isInArray(_ghoulCorpses, item2.itemid) == TRUE) then
		
			creatureToSummon = "Reborn Ghoul"
		else
		
			return TRUE
		end
		
		if (math.random(1, 10) <= 4) then
		
			doPlayerAddItem(cid, CUSTOM_ITEMS.DARK_DUST, 1)
			doSendMagicEffect(getThingPos(item2.uid), CONST_ME_HOLYAREA)
			doPlayerSay(cid, "Esta pobre alma agora encontrou a luz!", TALKTYPE_ORANGE_1)
			doRemoveItem(item2.uid)
			
		else
		
			doSummonCreature(creatureToSummon, getThingPos(item2.uid))
			doSendMagicEffect(getThingPos(item2.uid), CONST_ME_MORTAREA)		
			doRemoveItem(item2.uid)
		end		
	end
end