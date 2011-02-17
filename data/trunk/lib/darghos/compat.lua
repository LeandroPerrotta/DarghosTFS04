-- compatibilidade entre OTServ e TFS

if(darghos_distro == DISTROS_TFS) then
	getTownIdByName = getTownId
	getTownNameById = getTownName
	doSetCreatureDirection = doCreatureSetLookDirection
	broadcastMessageEx = broadcastMessage
	debugPrint = print
	doPlayerAddBless = doPlayerAddBlessing
	doPlayerAddHealth = doCreatureAddHealth
	getPlayerBless = getPlayerBlessing
	doPlayerAddBless = doPlayerAddBlessing
	doPlayerAddManaSpent = doPlayerAddSpentMana
	
	function setExperienceRate(cid, value)
		return doPlayerSetRate(cid, LEVEL_EXPERIENCE, value)
	end
	
	function setMagicRate(cid, value)
		return doPlayerSetRate(cid, LEVEL_MAGIC, value)
	end
	
	function setSkillRate(cid, skillid, value)
		return doPlayerSetRate(cid, skillid, value)
	end	
	
	function doPlayerRemoveSkillLossPercent(cid, amount)
		local lossvalue = getPlayerLossPercent(cid, PLAYERLOSS_EXPERIENCE)
		local newvalue = lossvalue - amount
		if newvalue < 0 then
			newvalue = 0
		end
		-- Setting experience is enough (all other types follow it)
		doPlayerSetLossPercent(cid, PLAYERLOSS_EXPERIENCE, newvalue)
	end
	
	function doPlayerUpdateItemLossPercent(cid)
		-- check quantity of bless
		local i = 0
		local blesses = 0
		while i < 5 do
			if getPlayerBless(cid, i) == TRUE then
				blesses = blesses + 1
			end
			i = i + 1
		end
	
		-- update %
		if blesses >= 5 then
			doPlayerSetLossPercent(cid, PLAYERLOSS_ITEMS, 0)
			doPlayerSetLossPercent(cid, PLAYERLOSS_CONTAINERS, 0)
		elseif blesses >= 4 then
			doPlayerSetLossPercent(cid, PLAYERLOSS_ITEMS, 1)
			doPlayerSetLossPercent(cid, PLAYERLOSS_CONTAINERS, 10)
		elseif blesses >= 3 then
			doPlayerSetLossPercent(cid, PLAYERLOSS_ITEMS, 3)
			doPlayerSetLossPercent(cid, PLAYERLOSS_CONTAINERS, 25)
		elseif blesses >= 2 then
			doPlayerSetLossPercent(cid, PLAYERLOSS_ITEMS, 5)
			doPlayerSetLossPercent(cid, PLAYERLOSS_CONTAINERS, 45)
		elseif blesses >= 1 then
			doPlayerSetLossPercent(cid, PLAYERLOSS_ITEMS, 7)
			doPlayerSetLossPercent(cid, PLAYERLOSS_CONTAINERS, 70)
		else
			doPlayerSetLossPercent(cid, PLAYERLOSS_ITEMS, 10)
			doPlayerSetLossPercent(cid, PLAYERLOSS_CONTAINERS, 100)
		end
	end	
	
	function getBlessPrice(level)
		local price = 2000
		level = math.min(120, level)
		if(level > 30) then
			price = (price + ((level - 30) * 200))
		end
	
		return price
	end	
	
	LEVEL_SKILL_FIST = SKILL_FIRST
	LEVEL_SKILL_CLUB = SKILL_CLUB
	LEVEL_SKILL_SWORD = SKILL_SWORD
	LEVEL_SKILL_AXE = SKILL_AXE
	LEVEL_SKILL_DISTANCE = SKILL_DISTANCE
	LEVEL_SKILL_SHIELDING = SKILL_SHIELD
	LEVEL_SKILL_FISHING = SKILL_FISHING
	LEVEL_MAGIC = SKILL__MAGLEVEL
	LEVEL_EXPERIENCE = SKILL__LEVEL	
	
	CONDITION_NONE = 0
	CONDITION_POISON = 1
	CONDITION_FIRE = 2
	CONDITION_ENERGY = 4
	CONDITION_LIFEDRAIN = 8
	CONDITION_HASTE = 16
	CONDITION_PARALYZE = 32
	CONDITION_OUTFIT = 64
	CONDITION_INVISIBLE = 128
	CONDITION_LIGHT = 256
	CONDITION_MANASHIELD = 512
	CONDITION_INFIGHT = 1024
	CONDITION_DRUNK = 2048
	CONDITION_EXHAUST_YELL = 4096
	CONDITION_FOOD = 8192
	CONDITION_REGENERATION = 8192
	CONDITION_SOUL = 16384
	CONDITION_DROWN = 32768
	CONDITION_MUTED = 65536
	CONDITION_ATTRIBUTES = 131072
	CONDITION_FREEZING = 262144
	CONDITION_DAZZLED = 524288
	CONDITION_CURSED = 1048576
	CONDITION_EXHAUST_COMBAT = 2097152
	CONDITION_EXHAUST_HEAL   = 4194304
	CONDITION_PACIFIED = 8388608
	CONDITION_HUNTING = 16777216
	CONDITION_TRADE_MUTED = 33554432
	CONDITION_EXHAUST_OTHERS = 67108864
	CONDITION_EXHAUST_POTION = CONDITION_EXHAUST_OTHERS 
	CONDITION_EXHAUSTED = CONDITION_EXHAUST_OTHERS	
	
elseif(darghos_distro == DISTROS_OPENTIBIA) then

	function doPlayerAddMagLevel(cid, count)
	
		for i = 1, count, 1 do
			local manaspent = 1600 * 1.1 ^ getPlayerMagLevel(cid) + 1	
			doPlayerAddManaSpent(cid, manaspent, TRUE)
		end
		
		return getPlayerMagLevel(cid)
	end
	
	function doPlayerAddSkill(cid, skillid, count)
		
		if(skillid == LEVEL_MAGIC or skillid == LEVEL_EXPERIENCE) then
			return false
		end
		
		for i = 1, count, 1 do
			if(doPlayerAddSkillTry(cid, skillid, getIntegerLimit(32, true), TRUE) == LUA_ERROR) then
				return false
			end
		end
		
		return true
	end
	
	function getIntegerLimit(bits, signed)
		local value = (signed and (math.pow(2, bits) / 2) or (math.pow(2, bits)))
		return (value - 1)
	end
end