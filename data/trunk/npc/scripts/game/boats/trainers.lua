local keywordHandler = KeywordHandler:new()
local npcHandler = NpcHandler:new(keywordHandler)
NpcSystem.parseParameters(npcHandler)

-- OTServ event handling functions start
function onCreatureAppear(cid)				npcHandler:onCreatureAppear(cid) end
function onCreatureDisappear(cid) 			npcHandler:onCreatureDisappear(cid) end
function onCreatureSay(cid, type, msg) 	npcHandler:onCreatureSay(cid, type, msg) end
function onThink() 						npcHandler:onThink() end
-- OTServ event handling functions end
    
boatDestiny.addAracura(keywordHandler, npcHandler, D_CustomNpcModules.travelTrainingIsland)
boatDestiny.addQuendor(keywordHandler, npcHandler, D_CustomNpcModules.travelTrainingIsland)    
boatDestiny.addSalazart(keywordHandler, npcHandler, D_CustomNpcModules.travelTrainingIsland)
boatDestiny.addAaragon(keywordHandler, npcHandler, D_CustomNpcModules.travelTrainingIsland)
boatDestiny.addNorthrend(keywordHandler, npcHandler, D_CustomNpcModules.travelTrainingIsland)
boatDestiny.addKashmir(keywordHandler, npcHandler, D_CustomNpcModules.travelTrainingIsland)  
        
keywordHandler:addKeyword({'passage'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = 'I can take you to Aracura, Quendor, Northrend, Aaragon, Kashmir and Salazart.'})
keywordHandler:addKeyword({'job'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = 'I am the captain of this ship.'})
keywordHandler:addKeyword({'travel'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = 'I can take you to Aracura, Quendor, Northrend, Aaragon, Kashmir and Salazart.'})

-- Makes sure the npc reacts when you say hi, bye etc.
npcHandler:addModule(FocusModule:new())