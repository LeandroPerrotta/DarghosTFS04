function onAdvance(cid, type, oldlevel, newlevel)
		
	if(type == LEVEL_EXPERIENCE) then
	
		setRateStage(cid, newlevel)
	else
		setSkillStageOnAdvance(cid, type, newlevel)
	end		
	
	return LUA_TRUE
end