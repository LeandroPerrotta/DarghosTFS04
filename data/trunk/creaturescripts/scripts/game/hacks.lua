local function checkLight(cid)
	local hackstate = getPlayerStorageValue(cid, sid.HACKS_LIGHT)
	
	if(hackstate == LIGHT_FULL) then
		setPlayerLight(cid, LIGHT_FULL)
	end	
end

local function checkCastMana(cid)

	local hackstate = getPlayerStorageValue(cid, sid.HACKS_CASTMANA)
	
	if(hackstate == 1) then
	
		local mana = getCreatureMana(cid)
		local manamax = getCreatureMaxMana(cid)
		local manachange = math.ceil(manamax / 2) -- default is half (50%)
		local manalimit = manamax - math.ceil(manamax / 4) -- 75%

		if(mana >= manalimit) then
			doPlayerAddMana(cid, -(manachange))
			doPlayerAddManaSpent(cid, manachange)
		end
	end
end

function onThink(cid, interval)
	if(not isCreature(cid)) then
		return
	end

	if(not darghos_need_eat) then
		if(getPlayerFood(cid) == 0) then
			doPlayerFeed(cid, 1200)
		end
	end

	checkLight(cid)
	checkCastMana(cid)
end
