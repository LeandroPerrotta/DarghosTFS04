local keywordHandler = KeywordHandler:new()
local npcHandler = NpcHandler:new(keywordHandler)
NpcSystem.parseParameters(npcHandler)

function onCreatureAppear(cid)            npcHandler:onCreatureAppear(cid)            end
function onCreatureDisappear(cid)        npcHandler:onCreatureDisappear(cid)            end
function onCreatureSay(cid, type, msg)    npcHandler:onCreatureSay(cid, type, msg)    end
function onThink()                        npcHandler:onThink()                        end

function task(cid, message, keywords, parameters, node)
    if(not npcHandler:isFocused(cid)) then
            return false
    end

	task = Task:new()
	task:loadById(CAP_ONE.QUENDOR.TRAVELER_IOP)
	task:setPlayer(cid)
	task:setNpcName(getNpcName())
	
	if(task:getState() ~= taskStats.COMPLETED and task:checkPlayerRequirements()) then
		npcHandler:say("Oh, claro! Mészáros havia me falado sobre você, o bravo matador de dragões, e pediu que lhe desse este premio por você ter o ajudado:",cid)
		task:doPlayerAddReward()			
		task:setCompleted()
		npcHandler:say("Ele havia dito que esperava que você gostasse. Deve ter o ajudado em algo muito perigoso, pois é uma generosa recompensa. Bom, se ver-lo por ai mande lembranças. Boa sorte bravo guerreiro!",cid)
	else
		npcHandler:say("Ever talk with citizens about tasks and missions, any times then can give alot of rewards!",cid)
	end

    return true
end

keywordHandler:addKeyword({'tarefa'}, task, nil)

npcHandler:addModule(FocusModule:new())