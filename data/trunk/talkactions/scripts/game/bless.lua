function onSay(cid, words, param)	
	local blesses = {
		{name="First", location="Quendor"},
		{name="Second", location="Thorn"},
		{name="Third", location="Aaragon"},
		{name="Fourth", location="Thaun"},
		{name="Fifth", location="Salazart"}
	}
	
	local totalBlesses = 0
	
	local message = "Here you can see if you have completed the blesses, or no.\n\n"
	
	for k,v in pairs(blesses) do
	
		if(getPlayerBless(cid, k)) then
			message = message .. v.name .. " (" .. v.location .. "): Completed\n"
			totalBlesses = totalBlesses + 1
		else
			message = message .. v.name .. " (" .. v.location .. "): n/a\n"
		end
	end
	
	message = message .. "\nPvP Bless (twist of fate): "
	
	if(getPlayerPVPBlessing(cid)) then	
		message = message .. "You have the PvP Bless. Your regular blessings are protected when you die in an duel and 40% or more of the total damage received are from another human player (not monsters)!"
	else
		message = message .. "You do not have the PvP Bless, buy it to protect your regular blessings in any temple NPC!"
	end
	
	message = message .. "\nItems drop: "
	
	if (totalBlesses == #blesses) then
		message = message .. "You have all blesses and your item/backpack loss is FULL SECURE!"
	else	
		message = message .. "You dont have all blesses and your item/back not is secure, and go drop when you die."
	end
	
	doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, message)	
end