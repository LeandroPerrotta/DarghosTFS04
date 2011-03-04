local GOLD_COINS = {2148, 2152, 2160}
local GOLD_RATE = 3

function autoloot(cid, target, pos)

	local moneyToAdd = 0
	local goldInCorpse = {}

    local function scanContainer(cid, uid)    
        for k = (getContainerSize(uid) - 1), 0, -1 do
            local tmp = getContainerItem(uid, k)
            
            if (isInArray(GOLD_COINS, tmp.itemid)) then	
            	
            	if(tmp.itemid == GOLD_COINS[1]) then
            		moneyToAdd = moneyToAdd + tmp.type
        		elseif(tmp.itemid == GOLD_COINS[2]) then
        			moneyToAdd = moneyToAdd + (tmp.type * 100)
        		elseif(tmp.itemid == GOLD_COINS[3]) then
        			moneyToAdd = moneyToAdd + (tmp.type * 10000)
            	end
            	
                table.insert(goldInCorpse, tmp.uid)
            elseif isContainer(tmp.uid) then
                scanContainer(cid, tmp.uid)
            end
        end      
    end
 
 	
    local items = {}
    for i = getTileInfo(pos).items, 1, -1 do
        pos.stackpos = i
        table.insert(items, getThingFromPos(pos))
    end

    if (#items == 0) then
        return
    end
 
    local corpses = {}
    for _, item in ipairs(items) do
    	if(isContainer(item.uid)) then
	        local name = getItemName(item.uid):lower()
	        
	        if (name:find("slain") or name:find("dead")) then
	            table.insert(corpses, item.uid)
	        end
        end
    end	
 
 	if(#corpses > 0) then
 		for k,corpse in pairs(corpses) do
	        scanContainer(cid, corpse)	
	    end
 	end
    
    if(moneyToAdd == 0) then
    	return
    end
    
  	local playerMoney = getPlayerMoney(cid)
	local totalMoney = playerMoney + (moneyToAdd * GOLD_RATE) 
    
	if(playerMoney > 0 and not doPlayerRemoveMoney(cid, playerMoney)) then
		print("[Autolot gold] Can not remove player previous money.")
		return
	end
	
	if(not doPlayerAddMoney(cid, totalMoney)) then
		print("[Autolot gold] Can not add new player money.")
		return
	end

	for k,v in pairs(goldInCorpse) do
		if(not doRemoveItem(v)) then
			print("[Autolot gold] Can not remove gold from corpse (uid: " .. v .. ")")
			return
		end
	end

    doPlayerSendTextMessage(cid, MESSAGE_EVENT_DEFAULT, 'All the gold coins found in the creature defeated with an plus extra bonus of ' .. (GOLD_RATE * 100) .. '% (' .. (GOLD_RATE * moneyToAdd) .. ' gps) has sent to their inventory.')      
end

function onKill(cid, target, lastHit)
    if not isPlayer(target) then
        addEvent(autoloot, 150, cid, getCreatureName(target), getCreaturePosition(target))
    end
    return true
end