local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_TYPE, COMBAT_PHYSICALDAMAGE)
setCombatParam(combat, COMBAT_PARAM_BLOCKARMOR, TRUE)
setCombatParam(combat, COMBAT_PARAM_DISTANCEEFFECT, CONST_ANI_WEAPONTYPE)
setCombatParam(combat, COMBAT_PARAM_USECHARGES, TRUE)

if(darghos_distro == DISTROS_TFS) then
	function getSpellDamage(cid, level, weaponSkill, weaponAttack, attackStrength)	
		local min = (((weaponSkill+weaponAttack)/3)+(level/5))
		local max = ((weaponSkill+weaponAttack)+(level/5))
	
		return -min, -max
	end
elseif(darghos_distro == DISTROS_OPENTIBIA) then
	function getSpellDamage(cid, weaponSkill, weaponAttack, attackStrength)
		local level = getPlayerLevel(cid)
	
		local min = (((weaponSkill+weaponAttack)/3)+(level/5))
		local max = ((weaponSkill+weaponAttack)+(level/5))
	
		return -min, -max
	end
end


setCombatCallback(combat, CALLBACK_PARAM_SKILLVALUE, "getSpellDamage")

function onCastSpell(cid, var)
	return doCombat(cid, combat, var)
end
