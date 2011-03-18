-----------
-- SUMMONER ADDON
-----------

addonSummoner = {}

function addonSummoner.changeTicket(keywordHandler, npcHandler)

	local params = {
		npcHandler = npcHandler,
		neededItems = {
			{anyOf = {{id = 7636}, {id = 7635}, {id = 7634}}, count = 100}
		},	
		receiveItems = {
			{id = 5957}
		},
		fail = "Sorry but if you want a lottery ticket you need give me 100 vials and you not have this...",
		success = "Great! I've signed you up for our bonus system. From now on, you will have the chance to win the potion belt addon!"
	}

	local node = keywordHandler:addKeyword({'ticket'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = 'Would you like to get a lottery ticket instead of the deposit for your vials?'})
	node:addChildKeyword({'yes'}, D_CustomNpcModules.addonTradeItems, params)
	node:addChildKeyword({'no'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, reset = true, text = 'Then stay here!'})
end

ADDON_ITEMS = {
	["summoner_changeticket"] = addonSummoner.changeTicket
}

-- Chama um modulo de AddonItems diretamente pelo XML do monstro
-- verificar a const ADDON_ITEMS
function parseAddonItemModule(keywordHandler, npcHandler)
	local item_module = NpcSystem.getParameter("call_addon_item")
	if(item_module ~= nil) then
		local addon_func = ADDON_ITEMS[item_module]
		addon_func(keywordHandler, npcHandler)
	end
end
