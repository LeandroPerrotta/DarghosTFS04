function D_onStartServer()
	local sendPlayerToTemple = getGlobalStorageValue(gid.SEND_PLAYERS_TO_TEMPLE)
	
	if(sendPlayerToTemple == 1) then
		db.executeQuery("UPDATE `players` SET `posx` = '0', `posy` = '0', `posz` = '0';")
		setGlobalStorageValue(gid.SEND_PLAYERS_TO_TEMPLE, 0)
		print("Sending players to temple.")
	end
end

function raidLog(raidname)
	local out = os.date("%X") .. " | Raid [" .. raidname .. "] started. "
	
	local date = os.date("*t")
	local fileStr = date.day .. "-" .. date.month .. ".log"
	local patch = getDataDir() .. "logs/raids/"
	local file = io.open(patch .. fileStr, "a+")
	
	file:write(out .. "\n")
	file:close()
end

function setPlayerAntiIdle(cid, interval)
	
	if(not isCreature(cid)) then
		return
	end
	
	if(interval > 0) then
		local dir = math.random(0, 3)
		doCreatureSetLookDirection(cid, dir)	
	
		local eventid = addEvent(setPlayerAntiIdle, interval, cid, interval)
		setPlayerStorageValue(cid, sid.HACKS_DANCE_EVENT, eventid)
	else
		local lastevent = getPlayerStorageValue(cid, sid.HACKS_DANCE_EVENT)
		stopEvent(lastevent)
		setPlayerStorageValue(cid, sid.HACKS_DANCE_EVENT, STORAGE_NULL)
	end
end

function getPlayerAntiIdle(cid)
	local idle = getPlayerStorageValue(cid, sid.HACKS_DANCE_EVENT)
	
	if(idle == STORAGE_NULL) then
		idle = false
	else
		idle = true
	end
	
	return idle
end

function setPlayerLight(cid, lightmode)

	if(lightmode == LIGHT_FULL) then
	
		local condition = createConditionObject(CONDITION_LIGHT)
		setConditionParam(condition, CONDITION_PARAM_LIGHT_COLOR, 215)
		setConditionParam(condition, CONDITION_PARAM_TICKS, 1000) --33 minutes(time in ms)
		setConditionParam(condition, CONDITION_PARAM_LIGHT_LEVEL, 255)	
			
		doAddCondition(cid, condition)
		
		setPlayerStorageValue(cid, sid.HACKS_LIGHT, LIGHT_FULL)
			
	elseif(lightmode == LIGHT_NONE) then
		doRemoveCondition(cid, CONDITION_LIGHT)
		setPlayerStorageValue(cid, sid.HACKS_LIGHT, LIGHT_NONE)
	end
end

function movementTileOnStepIn(cid, item, position, fromPosition)

	if(item.itemid == 11062 or item.itemid == 11063) then
		doUpdateCreatureImpassable(cid)
	end

	return false
end

function movementTileOnStepOut(cid, item, position, fromPosition)

	-- special depot tile (non walkover)[
	if(item.itemid == 11062 or item.itemid == 11063 ) then
		doUpdateCreaturePassable(cid)
	end
	
	return false
end

function doUpdateCreatureImpassable(cid)
	
	if(getPlayerTown(cid) ~= towns.ISLAND_OF_PEACE) then
		return
	end
	
	doPlayerSetGroupId(cid, GROUP_PLAYER)
end

function doUpdateCreaturePassable(cid)

	if(getPlayerTown(cid) ~= towns.ISLAND_OF_PEACE) then
		return
	end
	
	doPlayerSetGroupId(cid, GROUP_PLAYER_NON_PVP)	
end

function getLuaFunctions()-- by Mock
	local str = ""
	for f,k in pairs(_G) do
		if type(k) == 'function' then
			str = str..f..','
		elseif type(k) == 'table' then
			for d,o in pairs(k) do
				if type(o) == 'function' then
					if f ~= '_G' and d ~= "_G" and f ~= 'package' then
						str = str..f.."."..d..','
					end
				elseif type(o) == 'table' then
					for m,n in pairs(o) do
						if type(n) == 'function' then
							if d == "_M" and m ~= "_M" and f ~= "_G" and f ~= 'package' then
								str = str..f.."."..m..","
							elseif f ~= '_G' and m ~= "_G" and d ~= "_G" and f ~= 'package' then
								str = str..f.."."..d..'.'..m..','
							end
						elseif type(n) == 'table' then
							for x,p in pairs(n) do
								if type(p) == 'function' then
									if m == "_M" and d ~= "_M" and f ~= "_G" and f ~= 'package' then
										str = str..f.."."..d..'.'..x..','
									elseif m == "_M" and d == "_M" and f ~= "_G" and f ~= 'package' then
										str = str..f.."."..x..','
									elseif m ~= "_M" and d == "_M" and f ~= "_G" and f ~= 'package' then
										str = str..f..'.'..m..'.'..x..','
									elseif f ~= '_G' and m ~= "_G" and d ~= "_G" and f ~= 'package' then
										str = str..f.."."..d..'.'..m..'.'..x..','
									end
								end
							end
						end
					end
				end
			end
		end
	end
	return string.explode(str,',')
end
 
function table.show(t, name, indent)
   local cart     -- a container
   local autoref  -- for self references

   --[[ counts the number of elements in a table
   local function tablecount(t)
      local n = 0
      for _, _ in pairs(t) do n = n+1 end
      return n
   end
   ]]
   -- (RiciLake) returns true if the table is empty
   local function isemptytable(t) return next(t) == nil end

   local function basicSerialize (o)
      local so = tostring(o)
      if type(o) == "function" then
         local info = debug.getinfo(o, "S")
         -- info.name is nil because o is not a calling level
         if info.what == "C" then
            return string.format("%q", so .. ", C function")
         else 
            -- the information is defined through lines
            return string.format("%q", so .. ", defined in (" ..
                info.linedefined .. "-" .. info.lastlinedefined ..
                ")" .. info.source)
         end
      elseif type(o) == "number" then
         return so
      else
         return string.format("%q", so)
      end
   end

   local function addtocart (value, name, indent, saved, field)
      indent = indent or ""
      saved = saved or {}
      field = field or name

      cart = cart .. indent .. field

      if type(value) ~= "table" then
         cart = cart .. " = " .. basicSerialize(value) .. ";\n"
      else
         if saved[value] then
            cart = cart .. " = {}; -- " .. saved[value] 
                        .. " (self reference)\n"
            autoref = autoref ..  name .. " = " .. saved[value] .. ";\n"
         else
            saved[value] = name
            --if tablecount(value) == 0 then
            if isemptytable(value) then
               cart = cart .. " = {};\n"
            else
               cart = cart .. " = {\n"
               for k, v in pairs(value) do
                  k = basicSerialize(k)
                  local fname = string.format("%s[%s]", name, k)
                  field = string.format("[%s]", k)
                  -- three spaces between levels
                  addtocart(v, fname, indent .. "   ", saved, field)
               end
               cart = cart .. indent .. "};\n"
            end
         end
      end
   end

   name = name or "__unnamed__"
   if type(t) ~= "table" then
      return name .. " = " .. basicSerialize(t)
   end
   cart, autoref = "", ""
   addtocart(t, name, indent)
   return cart .. autoref
end 

function consoleLog(type, npcname, caller, string, params)
	local out = os.date("%X") .. " | [" .. type .. "] " .. caller .. " | " .. string
	
	if(params ~= nil) then
		out = out .. " | Params: {"
		
		local isFirst = true	
		
		for k,v in pairs(params) do
			
			if(not isFirst) then
				out = out .. ", "
			end
			
			out = out .. "[" .. k .. "] = " .. v
			
			isFirst = false
		end
		
		out = out .. "}"
	end
	
	local printTypes = { T_LOG_ALL }
	
	if(isInArray(printTypes, type) == TRUE or printTypes[1] == T_LOG_ALL) then
	
		local date = os.date("*t")
		local fileStr = npcname .. "_" .. date.day .. "-" .. date.month .. ".log"
		local patch = getDataDir() .. "logs/npc/"
		local file = io.open(patch .. fileStr, "a+")
		
		file:write(out .. "\n")
		file:close()
		
		--debugPrint(out)
	end
end

function getPlayerHighMelee(cid)
	local skill = getPlayerSkill(cid, LEVEL_SKILL_CLUB)
	local skillid = LEVEL_SKILL_CLUB
	
	if(getPlayerSkill(cid, LEVEL_SKILL_SWORD) > skill) then
		skillid = LEVEL_SKILL_SWORD
		skill = getPlayerSkill(cid, LEVEL_SKILL_SWORD)
	end
	
	if(getPlayerSkill(cid, LEVEL_SKILL_AXE) > skill) then
		skillid = LEVEL_SKILL_AXE
		skill = getPlayerSkill(cid, LEVEL_SKILL_AXE)
	end
	
	return skillid
end

function startShieldTrain(cid, target)
	
	local trainingShield = getPlayerStorageValue(cid, sid.TRAINING_SHIELD) > 0 and true or false

	if(not trainingShield) then
		addEvent(addShieldTrie, 1000 * 2, cid, target)
		setPlayerStorageValue(cid, sid.TRAINING_SHIELD, 1)
	end
end

function addShieldTrie(cid, target)	

	-- aqui provavelmente o player morreu
	if(isCreature(cid) == FALSE) then
		return
	end

	--print("Training: " .. getCreatureName(cid) .. " value: " .. getPlayerStorageValue(cid, sid.TRAINING_SHIELD))
	local cTarget = getCreatureTarget(cid)
	
	if(cTarget == 0) then
		--print("Alvo não encontrado, limpando... ")
		setPlayerStorageValue(cid, sid.TRAINING_SHIELD, 0)
		return
	else 
	
		if(getCreatureName(cTarget) ~= "Marksman Target" and getCreatureName(cTarget) ~= "Hitdoll") then
			setPlayerStorageValue(cid, sid.TRAINING_SHIELD, 0)
			return
		end
		
		doPlayerAddSkillTry(cid, LEVEL_SKILL_SHIELDING, 2, TRUE) 
		doSendMagicEffect(getPlayerPosition(cid), CONST_ME_POFF)
		
		addEvent(addShieldTrie, 1000 * 2, cid, target)			
	end	
end

function addAllOufits(cid)

	if(isPlayer(cid) == TRUE) then
	
		doPlayerAddOutfit(cid, outfits.CITIZEN.male, 3)
		doPlayerAddOutfit(cid, outfits.CITIZEN.female, 3)
		
		doPlayerAddOutfit(cid, outfits.HUNTER.male, 3)
		doPlayerAddOutfit(cid, outfits.HUNTER.female, 3)
		
		doPlayerAddOutfit(cid, outfits.MAGE.male, 3)
		doPlayerAddOutfit(cid, outfits.MAGE.female, 3)
		
		doPlayerAddOutfit(cid, outfits.KNIGHT.male, 3)
		doPlayerAddOutfit(cid, outfits.KNIGHT.female, 3)
		
		doPlayerAddOutfit(cid, outfits.NOBLE.male, 3)
		doPlayerAddOutfit(cid, outfits.NOBLE.female, 3)
		
		doPlayerAddOutfit(cid, outfits.SUMMONER.male, 3)
		doPlayerAddOutfit(cid, outfits.SUMMONER.female, 3)
		
		doPlayerAddOutfit(cid, outfits.WARRIOR.male, 3)
		doPlayerAddOutfit(cid, outfits.WARRIOR.female, 3)
		
		doPlayerAddOutfit(cid, outfits.BARBARIAN.male, 3)
		doPlayerAddOutfit(cid, outfits.BARBARIAN.female, 3)
		
		doPlayerAddOutfit(cid, outfits.DRUID.male, 3)
		doPlayerAddOutfit(cid, outfits.DRUID.female, 3)
		
		doPlayerAddOutfit(cid, outfits.WIZARD.male, 3)
		doPlayerAddOutfit(cid, outfits.WIZARD.female, 3)
		
		doPlayerAddOutfit(cid, outfits.ORIENTAL.male, 3)
		doPlayerAddOutfit(cid, outfits.ORIENTAL.female, 3)
		
		doPlayerAddOutfit(cid, outfits.PIRATE.male, 3)
		doPlayerAddOutfit(cid, outfits.PIRATE.female, 3)
		
		doPlayerAddOutfit(cid, outfits.ASSASSIN.male, 3)
		doPlayerAddOutfit(cid, outfits.ASSASSIN.female, 3)
		
		doPlayerAddOutfit(cid, outfits.BEGGAR.male, 3)
		doPlayerAddOutfit(cid, outfits.BEGGAR.female, 3)
		
		doPlayerAddOutfit(cid, outfits.SHAMAN.male, 3)
		doPlayerAddOutfit(cid, outfits.SHAMAN.female, 3)
		
		doPlayerAddOutfit(cid, outfits.NORSE.male, 3)
		doPlayerAddOutfit(cid, outfits.NORSE.female, 3)
		
		doPlayerAddOutfit(cid, outfits.NIGHTMARE.male, 3)
		doPlayerAddOutfit(cid, outfits.NIGHTMARE.female, 3)
		
		doPlayerAddOutfit(cid, outfits.JESTER.male, 3)
		doPlayerAddOutfit(cid, outfits.JESTER.female, 3)
		
		doPlayerAddOutfit(cid, outfits.BROTHERHOOD.male, 3)
		doPlayerAddOutfit(cid, outfits.BROTHERHOOD.female, 3)
		
		doPlayerAddOutfit(cid, outfits.DEMONHUNTER.male, 3)
		doPlayerAddOutfit(cid, outfits.DEMONHUNTER.female, 3)
		
		doPlayerAddOutfit(cid, outfits.YALAHARIAN.male, 3)
		doPlayerAddOutfit(cid, outfits.YALAHARIAN.female, 3)
		
		doPlayerAddOutfit(cid, outfits.WARMASTER.male, 3)
		doPlayerAddOutfit(cid, outfits.WARMASTER.female, 3)
		
		doPlayerAddOutfit(cid, outfits.WEEDING.male, 3)
		doPlayerAddOutfit(cid, outfits.WEEDING.female, 3)
	end
end

--[[
	* REGISTRO DE EVENTOS ONKILL PARA MISS�ES
]]--
function OnKillCreatureMission(cid)

	-- Bonartes Mission's
	local _demonMission = getPlayerStorageValue(cid, QUESTLOG.MISSION_BONARTES.KILL_DEMONS)
	local _heroMission = getPlayerStorageValue(cid, QUESTLOG.MISSION_BONARTES.KILL_HEROS)
	local _behemothMission = getPlayerStorageValue(cid, QUESTLOG.MISSION_BONARTES.KILL_BEHEMOTHS)	
	
	if(_heroMission == 2 or _behemothMission == 1 or _demonMission == 1) then
		registerCreatureEvent(cid, "CustomBonartesTasks")
	end
end

--[[
	* DIVINE ANKH QUEST
]]--
function onLordVankynerDie()

	local door = getThing(uid.CHURCH_CHAMBER_DOOR)
	
	doSetItemActionId(door.uid, 100)
	
	addEvent(LordVankynerEvent, 1000 * 60 * 10)		
end

function LordVankynerEvent()

	local door = getThing(uid.CHURCH_CHAMBER_DOOR)
	
	doSetItemActionId(door.uid, 10000)
	
	local ALTAR_ID = 1643
	
	local altar = doCreateItem(ALTAR_ID, 1, mcord.CHURCH_ALTAR)
	doSetItemActionId(altar, aid.CHURCH_ALTAR)
	
	summonLordVankyner()
end

function summonLordVankyner()

	local creaturePos = getThingPos(uid.LORD_VANKYNER)
	local creature = doSummonCreature("Lord Vankyner", creaturePos)
	registerCreatureEvent(creature, "CreatureDie")
end

--[[
	* ARIADNE QUEST
]]--
function onGhazranDie(corpse)

	doSetItemActionId(corpse, aid.ARIADNE_GHAZRAN_CORPSE)
end

function obsidianKnifeOnGhazranCorpse(cid, corpse)

	local hasRemovedTongue = (getPlayerStorageValue(cid, sid.ARIADNE_GHAZRAN_TONGUE) == 1)
	
	if not(hasRemovedTongue) then

		doPlayerSendTextMessage(cid, MESSAGE_INFO_DESCR, "Voc� conseguiu obter a l�ngua de Ghazran. Seu questlog foi atualizado.")
		setPlayerStorageValue(cid, sid.ARIADNE_GHAZRAN_TONGUE, 1)
		setPlayerStorageValue(cid, QUESTLOG.ARIADNE.GHAZRAN_WING, 3)
	else
		doPlayerSendCancel(cid, "Voc� j� obteve a l�ngua de Ghazran.")
	end
end

--[[
	@ Chama scripts customizados para quests em chests.lua
]]--
function chestScripts(cid, questActionId)

	if(questActionId == aid.CHEST_DIVINE_ANKH) then
		setPlayerStorageValue(cid, QUESTLOG.DIVINE_ANKH.CHAMBER_TEMPTATION, 4)
	end
end

--[[
	@ Fun��o chamada quando o server � iniciado
]]--
function onServerStart()

	Dungeons.onServerStart()
	
	summonLordVankyner()
end


--[[
	* GLOBAL SERVER SAVE
]]--
function startGlobalSave()
	
	broadcastMessage("Efetuando pause para auto save...", MESSAGE_STATUS_CONSOLE_RED)
	print("[autosave] Auto save iniciando...")
	addEvent(endGlobalSave, 500)
	
end

function endGlobalSave()

	if(doSaveServer(TRUE) ~= LUA_ERROR) then
		broadcastMessage("Auto save concluido.", MESSAGE_STATUS_CONSOLE_RED)
		print("[autosave] Auto save concluido.")
	else
		print("[saveserver] Auto save falhou.")
	end
end

--[[
	* TRAINERS
]]--
function addTrainer(actionid)

	if(actionid == aid.TRAINERS_WEST) then
		local pos = {
			trainer1 = {x= item.x-1, y= item.y-1, z= item.z},
			trainer2 = {x= item.x-1, y= item.y+1, z= item.z}				
		}
	end
	
	if(actionid == aid.TRAINERS_EAST) then
		local pos = {
			trainer1 = {x= item.x-1, y= item.y-1, z= item.z},
			trainer2 = {x= item.x-1, y= item.y+1, z= item.z}				
		}	
	end
	
	

end

--[[
	* SETAR NOVOS ITEMS PARA FIRST LOGIN
]]--
function defineFirstItems(cid)

	if(isPlayer(cid) == FALSE) then
		return
	end

	if(getPlayerStorageValue(cid, sid.FIRSTLOGIN_ITEMS) == 1) then
		return
	end		
	
	-- general itens for all vocations
	local item_legs = doCreateItemEx(getItemIdByName("studded legs"), 1)
	local item_armor = 0
	local item_boots = doCreateItemEx(getItemIdByName("leather boots"), 1)
	local item_helmet = 0
	local item_left_hand = 0
	local item_right_hand = doCreateItemEx(getItemIdByName("dwarven shield"), 1)
	local item_backpack = doCreateItemEx(1988, 1)

	doAddContainerItem(item_backpack, 2120, 1) -- rope
	doAddContainerItem(item_backpack, 2554, 1) -- shovel	
	doAddContainerItem(item_backpack, 2789, 100) -- brown mushroom
	doAddContainerItem(item_backpack, 2152, 2) -- platinum coin
	doAddContainerItem(item_backpack, CUSTOM_ITEMS.TELEPORT_RUNE, 1) -- teleport rune
	
	if(isSorcerer(cid)) then
		item_armor = doCreateItemEx(getItemIdByName("magician's robe"), 1)
		item_helmet = doCreateItemEx(getItemIdByName("mage hat"), 1)
		item_left_hand = doCreateItemEx(getItemIdByName("wand of vortex"), 1)
	elseif(isDruid(cid)) then
		item_armor = doCreateItemEx(getItemIdByName("magician's robe"), 1)
		item_helmet = doCreateItemEx(getItemIdByName("mage hat"), 1)
		item_left_hand = doCreateItemEx(getItemIdByName("snakebite rod"), 1)
	elseif(isPaladin(cid)) then
		item_armor = doCreateItemEx(getItemIdByName("chain armor"), 1)
		item_helmet = doCreateItemEx(getItemIdByName("studded helmet"), 1)
		item_left_hand = doCreateItemEx(getItemIdByName("spear"), 5)
	elseif(isKnight(cid)) then
		item_armor = doCreateItemEx(getItemIdByName("chain armor"), 1)
		item_helmet = doCreateItemEx(getItemIdByName("studded helmet"), 1)
		item_left_hand = doCreateItemEx(getItemIdByName("hatchet"), 1)	
		
		-- adicional weapon to knights choose in backpack
		doAddContainerItem(item_backpack, getItemIdByName("katana"), 1) 
		doAddContainerItem(item_backpack, getItemIdByName("mace"), 1)
	end

	doPlayerAddItemEx(cid, item_legs, FALSE, CONST_SLOT_LEGS)
	doPlayerAddItemEx(cid, item_armor, FALSE, CONST_SLOT_ARMOR)
	doPlayerAddItemEx(cid, item_boots, FALSE, CONST_SLOT_FEET)
	doPlayerAddItemEx(cid, item_helmet, FALSE, CONST_SLOT_HEAD)
	doPlayerAddItemEx(cid, item_left_hand, FALSE, CONST_SLOT_LEFT)
	doPlayerAddItemEx(cid, item_right_hand, FALSE, CONST_SLOT_RIGHT)
	doPlayerAddItemEx(cid, item_backpack, FALSE, CONST_SLOT_BACKPACK)
	
	setPlayerStorageValue(cid, sid.FIRSTLOGIN_ITEMS, 1)		
end

function setRateStage(cid, newlevel)
	
	local stages = {
		normal = {
			{end_level = 49, multipler = 50}, 
			{start_level = 50, end_level = 99, multipler = 25}, 
			{start_level = 100, end_level = 119, multipler = 15}, 
			{start_level = 120, end_level = 139, multipler = 10}, 
			{start_level = 140, end_level = 159, multipler = 5}, 
			{start_level = 160, end_level = 179, multipler = 2}, 
			{start_level = 180, end_level = 199, multipler = 1.5}, 
			{start_level = 200, end_level = 239, multipler = 1}, 
			{start_level = 240, multipler = 0.5}, 
		},
		
		secure = {
			{end_level = 49, multipler = 50}, 
			{start_level = 50, end_level = 79, multipler = 25}, 
			{start_level = 80, multipler = 1}, 			
		}
	}
	
	local function readStagesNode(node, cid, newlevel)
		for k,v in pairs(node) do
		
			if(v.end_level == nil) then
				v.end_level = 999
			end
			
			if(v.start_level == nil) then
				v.start_level = 1
			end
		
			if(newlevel >= v.start_level and newlevel <= v.end_level) then
				--print("[" .. getCreatureName(cid) .. "] level " .. newlevel .. ", exp stage " .. v.multipler .. "x")
				setExperienceRate(cid, v.multipler)
				break
			end	
		end
	end
	
	local stageNode = stages.normal
	
	if(getPlayerTown(cid) == towns.ISLAND_OF_PEACE) then
		stageNode = stages.secure
	end
	
	readStagesNode(stageNode, cid, newlevel)
	
	return LUA_TRUE

end

function setLoginSkillsRateStage(cid)

	local skills = {
		normal = {
			{end_level = 79, multipler = 80}, 
			{start_level = 80, end_level = 89, multipler = 50}, 
			{start_level = 90, end_level = 99, multipler = 25}, 
			{start_level = 100, end_level = 109, multipler = 10}, 
			{start_level = 110, multipler = 5}
		}
	}
	
	local magic = {
		["mages"] = {
			{end_level = 59, multipler = 15}, 
			{start_level = 60, end_level = 79, multipler = 10}, 
			{start_level = 80, end_level = 89, multipler = 5}, 
			{start_level = 90, end_level = 95, multipler = 3}, 
			{start_level = 96, multipler = 1}		
		},
		
		["knights"] = {
			{end_level = 9, multipler = 15}, 
			{start_level = 10, end_level = 11, multipler = 7}, 
			{start_level = 12, multipler = 3}		
		},
		
		["paladins"] = {
			{end_level = 29, multipler = 15}, 
			{start_level = 30, end_level = 31, multipler = 7}, 
			{start_level = 32, multipler = 3}		
		}		
	}	

	local function readStagesNode(node, cid, newlevel, skillid)
		for k,v in pairs(node) do
		
			if(v.end_level == nil) then
				v.end_level = 999
			end
			
			if(v.start_level == nil) then
				v.start_level = 0
			end
		
			if(newlevel >= v.start_level and newlevel <= v.end_level) then
				--print("[" .. getCreatureName(cid) .. "] skill type: " .. skillid .. ", level " .. newlevel .. ", stage " .. v.multipler .. "x")
				setSkillRate(cid, skillid, v.multipler)
				break
			end	
		end
	end

	readStagesNode(skills.normal, cid, getPlayerSkillLevel(cid, LEVEL_SKILL_FIST), LEVEL_SKILL_FIST)
	readStagesNode(skills.normal, cid, getPlayerSkillLevel(cid, LEVEL_SKILL_CLUB), LEVEL_SKILL_CLUB)
	readStagesNode(skills.normal, cid, getPlayerSkillLevel(cid, LEVEL_SKILL_SWORD), LEVEL_SKILL_SWORD)
	readStagesNode(skills.normal, cid, getPlayerSkillLevel(cid, LEVEL_SKILL_AXE), LEVEL_SKILL_AXE)
	readStagesNode(skills.normal, cid, getPlayerSkillLevel(cid, LEVEL_SKILL_DISTANCE), LEVEL_SKILL_DISTANCE)
	readStagesNode(skills.normal, cid, getPlayerSkillLevel(cid, LEVEL_SKILL_SHIELDING), LEVEL_SKILL_SHIELDING)
	readStagesNode(skills.normal, cid, getPlayerSkillLevel(cid, LEVEL_SKILL_FISHING), LEVEL_SKILL_FISHING)
	
	if(isDruid(cid) or isSorcerer(cid)) then
		readStagesNode(magic["mages"], cid, getPlayerMagLevel(cid), LEVEL_MAGIC)	
	elseif(isKnight(cid)) then
		readStagesNode(magic["knights"], cid, getPlayerMagLevel(cid), LEVEL_MAGIC)	
	elseif(isPaladin(cid)) then
		readStagesNode(magic["paladins"], cid, getPlayerMagLevel(cid), LEVEL_MAGIC)	
	end
end

function setSkillStageOnAdvance(cid, skillid, newlevel)

	local skills = {
		normal = {
			{end_level = 79, multipler = 80}, 
			{start_level = 80, end_level = 89, multipler = 50}, 
			{start_level = 90, end_level = 99, multipler = 25}, 
			{start_level = 100, end_level = 109, multipler = 10}, 
			{start_level = 110, multipler = 5}
		}
	}
	
	local magic = {
		["mages"] = {
			{end_level = 59, multipler = 15}, 
			{start_level = 60, end_level = 79, multipler = 10}, 
			{start_level = 80, end_level = 89, multipler = 5}, 
			{start_level = 90, end_level = 95, multipler = 3}, 
			{start_level = 96, multipler = 1}		
		},
		
		["knights"] = {
			{end_level = 9, multipler = 15}, 
			{start_level = 10, end_level = 11, multipler = 7}, 
			{start_level = 12, multipler = 3}		
		},
		
		["paladins"] = {
			{end_level = 29, multipler = 15}, 
			{start_level = 30, end_level = 31, multipler = 7}, 
			{start_level = 32, multipler = 3}		
		}		
	}

	local function readStagesNode(node, cid, newlevel, skillid)
		for k,v in pairs(node) do
		
			if(v.end_level == nil) then
				v.end_level = 999
			end
			
			if(v.start_level == nil) then
				v.start_level = 0
			end
		
			if(newlevel >= v.start_level and newlevel <= v.end_level) then
				--print("[" .. getCreatureName(cid) .. "] skill type: " .. skillid .. ", level " .. newlevel .. ", stage " .. v.multipler .. "x")
				setSkillRate(cid, skillid, v.multipler)
				break
			end	
		end
	end
	
	if(skillid == LEVEL_MAGIC) then
		if(isDruid(cid) or isSorcerer(cid)) then
			readStagesNode(magic["mages"], cid, getPlayerMagLevel(cid), LEVEL_MAGIC)	
		elseif(isKnight(cid)) then
			readStagesNode(magic["knights"], cid, getPlayerMagLevel(cid), LEVEL_MAGIC)	
		elseif(isPaladin(cid)) then		
			readStagesNode(magic["paladins"], cid, getPlayerMagLevel(cid), LEVEL_MAGIC)	
		end	
	else
		readStagesNode(skills.normal, cid, getPlayerSkillLevel(cid, skillid), skillid)
	end
end

function playerRecord()

	if(not darghos_use_record) then
		return
	end

	local record = getGlobalStorageValue(gid.PLAYERS_RECORD)
	
	if(record ~= -1) then
		
		local playerson = getPlayersOnlineList()
		local total = #playerson
		
		if(total <= 50) then
			total = total * 2
		else
			total = total + 50
		end
		
		if(total > record) then
		
			setGlobalStorageValue(gid.PLAYERS_RECORD, total)
			broadcastMessage("A marca de ".. total .." jogadores online é um novo recorde no Darghos!", MESSAGE_EVENT_DEFAULT)
		end
	else

		setGlobalStorageValue(gid.PLAYERS_RECORD, 200)
	end
end

function msgcontains(txt, str)
      return (string.find(txt, str) and not string.find(txt, '(%w+)' .. str) and not string.find(txt, str .. '(%w+)'))
end

function checkGeneralInfoPlayer(cid)
	
	local level 		= 	getPlayerLevel(cid)
	
	if(isSorcerer(cid)) or (isDruid(cid)) then
		realHP 	=	(level * 5 + 145)
	elseif(isKnight(cid)) then
		realHP	=	(level * 15 + 65)
	elseif(isPaladin(cid)) then
		realHP	= 	(level * 10 + 105)
	else
		realHP	= 	(level * 5 + 145)
	end
	
	if(getPlayerMaxHealth(cid)) < realHP then
		print("[infoChecker] Player "..getCreatureName(cid).." esta com a life bugada!")
	else
		print("[infoChecker] Player "..getCreatureName(cid).." esta mil grau.")
	end	
	
end


-- Verifica��o ATUAL se um player est� em Area premmy, e teleporta ele para area free.
function runPremiumSystem(cid)

	local name = getCreatureName(cid)
	if(getPlayerTown(cid) == getTownIdByName("Island of Peace")) then
		backCity = "Island of Peace"
	else
		backCity = "Quendor City"
	end	
	local message = "Dear "..name..",\nYour premium time is over!\nYou were automatically taken to the temple of "..backCity..".\nContinue contributing to the Darghos Server and have a good game!\n\nYours,\nUltraXSoft Team."
	
	if isPremium(cid) == TRUE then
		
		if getPlayerStorageValue(cid,sid.PREMMY_VERIFY) ~= 1 then
		
			setPlayerStorageValue(cid, sid.PREMMY_VERIFY,1)
			doSendMagicEffect(getPlayerPosition(cid),11)
			
			--print("[premiumsystem] Configurou o Premium Status no Storage Value (Player:"..name..")")
		end
	else
	
		if getPlayerStorageValue(cid, sid.PREMMY_VERIFY) == 1 then
			
			if(getPlayerTown(cid) == getTownIdByName("Island of Peace")) then
				TP_CITY = ISLAND_PEACE
				ID_NAME = "Island of Peace"
			else
				TP_CITY = QUENDOR
				ID_NAME = "quendor"	
			end
			
			doPlayerSendTextMessage(cid, MESSAGE_EVENT_ADVANCE, message)
			doTeleportThing(cid, TP_CITY)
			setPlayerStorageValue(cid, sid.PREMMY_VERIFY,0)
			doPlayerSetTown(cid, getTownIdByName(ID_NAME))
			--Player is not premium - remove premium privileges
			--Change outfit
			local lookType = 128
			if(getPlayerSex(cid) == 0) then
				lookType = 136
			end
			doCreatureChangeOutfit(cid, {lookType = lookType, lookHead = 78, lookBody = 69, lookLegs = 97, lookFeet = 95, lookAddons = 0})
						
		end
	end
end

-- Reproduz um efeito em torno do jogador
function sendEnvolveEffect(cid, effect)

	doSendMagicEffect(getPlayerPosition(cid), effect)
	doSendMagicEffect({x = getPlayerPosition(cid).x + 1, y = getPlayerPosition(cid).y + 1, z = getPlayerPosition(cid).z}, effect) 
	doSendMagicEffect({x = getPlayerPosition(cid).x - 1, y = getPlayerPosition(cid).y + 1, z = getPlayerPosition(cid).z}, effect)
	doSendMagicEffect({x = getPlayerPosition(cid).x + 1, y = getPlayerPosition(cid).y - 1, z = getPlayerPosition(cid).z}, effect)
	doSendMagicEffect({x = getPlayerPosition(cid).x - 1, y = getPlayerPosition(cid).y - 1, z = getPlayerPosition(cid).z}, effect) 
	doSendMagicEffect({x = getPlayerPosition(cid).x, y = getPlayerPosition(cid).y - 1, z = getPlayerPosition(cid).z}, effect) 
	doSendMagicEffect({x = getPlayerPosition(cid).x - 1, y = getPlayerPosition(cid).y, z = getPlayerPosition(cid).z}, effect)
	doSendMagicEffect({x = getPlayerPosition(cid).x, y = getPlayerPosition(cid).y + 1, z = getPlayerPosition(cid).z}, effect)
	doSendMagicEffect({x = getPlayerPosition(cid).x + 1, y = getPlayerPosition(cid).y, z = getPlayerPosition(cid).z}, effect) 	
end 

-- Verifica se o player possui um item no shop a receber
-- Usado em creaturescripts/login.lua
function checkItemShop(cid)

	local shop_id = getPlayerStorageValue(cid, sid.SHOPSYS_ID)
	
	if shop_id ~= -1 then
		if(getPlayerFreeCap(cid) > 350) then
			local shop_itemid = getPlayerStorageValue(cid, sid.SHOPSYS_ITEM_ID)
			local shop_itemcount = getPlayerStorageValue(cid, sid.SHOPSYS_ITEM_COUNT)
			
			local presentBoxShop = doPlayerAddItem(cid, 1990, 1)
			local addContainer = doAddContainerItem(presentBoxShop, shop_itemid, shop_itemcount)
			
				if addContainer == LUA_ERROR then
					print("[itemshopsys] Item falhou ao ser adicionado (shopid: " .. shop_id .. ")")
					doPlayerSendTextMessage(cid, MESSAGE_EVENT_ADVANCE, "Ouve um erro ao entregar seu item, por favor, entre em contato com um Tutor ou Gamemaster.")
					
					setPlayerStorageValue(cid, sid.SHOPSYS_ID, 0)
					return
				end
				
				sendEnvolveEffect(cid, CONST_ME_ENERGYHIT)
			
				setPlayerStorageValue(cid, sid.SHOPSYS_ID, -1)
				setPlayerStorageValue(cid, sid.SHOPSYS_ITEM_ID, -1)
				setPlayerStorageValue(cid, sid.SHOPSYS_ITEM_COUNT, -1)
				
				doPlayerSendTextMessage(cid, MESSAGE_EVENT_ADVANCE, "You received in your inventory the item purchased in our Item Shop with success!")
				
		else
			doPlayerSendTextMessage(cid, MESSAGE_EVENT_ADVANCE, "You don't have capacity needed to receive the item purchased in our Item Shop. Please release 350oz and re-log in to receive the item.")
			--print("[itemshopsys] Nao possui cap sulficiente (shopid: " .. idFromShop .. ")")
		end
	end 
	
end