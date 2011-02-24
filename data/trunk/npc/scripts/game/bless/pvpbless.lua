local keywordHandler = KeywordHandler:new()
local npcHandler = NpcHandler:new(keywordHandler)
NpcSystem.parseParameters(npcHandler)


function onCreatureAppear(cid)				npcHandler:onCreatureAppear(cid) 			end
function onCreatureDisappear(cid) 			npcHandler:onCreatureDisappear(cid) 		end
function onCreatureSay(cid, type, msg) 		npcHandler:onCreatureSay(cid, type, msg) 	end
function onThink() 							npcHandler:onThink() 						end

local node1 = keywordHandler:addKeyword({'twist of fate'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = 'The pvp bless (twist of fate) will not reduce the death penality like the other blessings, but instead prevent you from losing your another regular blessings when you die from others (but not by monsters).  Do you want to receive that protection for a sacrifice of 2000 and extra 100 gold for each level between 30 and 270?'})
	node1:addChildKeyword({'yes'}, D_CustomNpcModules.pvpBless, {npcHandler = npcHandler, baseCost = 2000, levelCost = 100, startLevel = 30, endLevel = 270})
	node1:addChildKeyword({'no'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, reset = true, text = 'Too expensive, eh?'})

npcHandler:addModule(FocusModule:new())
