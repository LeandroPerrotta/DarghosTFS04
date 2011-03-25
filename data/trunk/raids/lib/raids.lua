local raids = {

	["demodras"] = {	
		respawns = {
			{centerx=1937, centery=2102, centerz=7, radius=5}, -- POH
			{centerx=2346, centery=1933, centerz=8, radius=5}, -- THORN DLAIR
			{centerx=3299, centery=1992, centerz=8, radius=5}, -- ARACURA DLAIR
			{centerx=2028, centery=2494, centerz=6, radius=5}, -- SALAZART DLAIR
			{centerx=2813, centery=1144, centerz=8, radius=5} -- AARAGON DLAIR		
		},
		
		monsters = {
			{name="Dragon Lord", quanty=2},
			{name="Demodras", quanty=1}
		}
	},
	
	["the old widow"] = {
		respawns = {
			{centerx=1956, centery=2041, centerz=10, radius=3}, -- POH (dentro do "buraco")
			{centerx=1934, centery=2121, centerz=7, radius=3}, -- POH (entrada para POI)
			{centerx=2296, centery=2277, centerz=9, radius=3} -- Tiquanda (na area escondida)
		},
		
		monsters = {
			{name="The Old Widow", quanty=1}
		}	
	},
	
	["the horned fox"] = {
		respawns = {
			{centerx=1830, centery=1942, centerz=10, radius=3}, -- Mintwallin
		},
		
		monsters = {
			{name="The Horned Fox", quanty=1}
		}	
	},
	
	["tiquandas revenge"] = {
		respawns = {
			{centerx=2356, centery=2269, centerz=7, radius=3},
			{centerx=2589, centery=2476, centerz=7, radius=3},
		},
		
		monsters = {
			{name="Tiquandas Revenge", quanty=1}
		}	
	}
}

local MAX_TRIES_PER_MONSTER = 10

function startRaid(raidName)
	
	local raidNode = raids[raidName]
	
	if(raidNode == nil) then
		print("[Raid Scripts Error] Raid " .. raidName .. " not found.")
		return true
	end	
	
	print("[Raid Scripts] " .. raidName .. " raid has started.")
	raidLog(raidName)
	
	local respawns = raidNode.respawns
	local monsters = raidNode.monsters
	local node = respawns[math.random(1, #respawns)]
	
	function getMonsterRaidPos(node)
	
		local spawnpos = {}
		
		spawnpos.x = math.random(node.centerx - node.radius, node.centerx + node.radius)
		spawnpos.y = math.random(node.centery - node.radius, node.centery + node.radius)
		spawnpos.z = node.centerz	
		
		return spawnpos
	end
	
	for k,v in pairs(monsters) do
	
		for i = 1, v.quanty, 1 do
		
			local spawnpos = getMonsterRaidPos(node)
		
			for x = 1, MAX_TRIES_PER_MONSTER, 1 do
				if(doCreateMonster(v.name, spawnpos, true)) then
					break
				end
			end		
		end
	end

	return true
end