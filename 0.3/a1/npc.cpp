//////////////////////////////////////////////////////////////////////
// OpenTibia - an opensource roleplaying game
//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//////////////////////////////////////////////////////////////////////
#include "otpch.h"

#include "definitions.h"
#include "npc.h"
#include "game.h"
#include "tools.h"
#include "configmanager.h"
#include "position.h"
#include "spells.h"

#include <algorithm>
#include <functional>
#include <string>
#include <sstream>
#include <fstream>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include "luascript.h"

extern ConfigManager g_config;
extern Game g_game;
extern Spells* g_spells;

AutoList<Npc> Npc::listNpc;

NpcScriptInterface* Npc::m_scriptInterface = NULL;

#ifdef __ENABLE_SERVER_DIAGNOSTIC__
uint32_t Npc::npcCount = 0;
#endif

void Npcs::reload()
{
	for(AutoList<Npc>::listiterator it = Npc::listNpc.list.begin(); it != Npc::listNpc.list.end(); ++it)
		it->second->closeAllShopWindows();

	delete Npc::m_scriptInterface;
	Npc::m_scriptInterface = NULL;

	for(AutoList<Npc>::listiterator it = Npc::listNpc.list.begin(); it != Npc::listNpc.list.end(); ++it)
		it->second->reload();
}

Npc* Npc::createNpc(const std::string& name)
{
	Npc* npc = new Npc(name);
	if(!npc)
		return NULL;

	if(!npc->load())
	{
		delete npc;
		return NULL;
	}
	return npc;
}

Npc::Npc(const std::string& _name) :
	Creature()
{
	m_filename = "data/npc/" + _name + ".xml";
	loaded = false;

	m_npcEventHandler = NULL;
	reset();

#ifdef __ENABLE_SERVER_DIAGNOSTIC__
	npcCount++;
#endif
}

Npc::~Npc()
{
	reset();

#ifdef __ENABLE_SERVER_DIAGNOSTIC__
	npcCount--;
#endif
}

bool Npc::load()
{
	if(isLoaded())
		return true;

	reset();

	if(!m_scriptInterface)
	{
		m_scriptInterface = new NpcScriptInterface();
		m_scriptInterface->loadNpcLib("data/npc/lib/npc.lua");
	}

	loaded = loadFromXml(m_filename);
	return isLoaded();
}

void Npc::reset()
{
	loaded = false;
	walkTicks = 1500;
	floorChange = false;
	attackable = false;
	hasBusyReply = false;
	hasScriptedFocus = false;
	focusCreature = 0;
	isIdle = true;
	talkRadius = 2;
	idleTime = 0;
	idleInterval = 5 * 60;
	defaultPublic = true;

	delete m_npcEventHandler;
	m_npcEventHandler = NULL;

	for(ResponseList::iterator it = responseList.begin(); it != responseList.end(); ++it)
		delete *it;

	for(StateList::iterator it = stateList.begin(); it != stateList.end(); ++it)
		delete *it;

	responseList.clear();
	stateList.clear();
	queueList.clear();
	m_parameters.clear();
	itemListMap.clear();
	responseScriptMap.clear();
	shopPlayerList.clear();
}

void Npc::reload()
{
	reset();
	load();

	//Simulate that the creature is placed on the map again.
	if(m_npcEventHandler)
		m_npcEventHandler->onCreatureAppear(this);

	if(walkTicks > 0)
		addEventWalk();
}

bool Npc::loadFromXml(const std::string& filename)
{
	xmlDocPtr doc = xmlParseFile(filename.c_str());
	if(doc)
	{
		xmlNodePtr root, p;
		root = xmlDocGetRootElement(doc);

		if(xmlStrcmp(root->name,(const xmlChar*)"npc") != 0)
		{
			std::cerr << "Malformed XML" << std::endl;
			return false;
		}

		int32_t intValue;
		std::string strValue;

		p = root->children;

		std::string scriptfile = "";
		if(readXMLString(root, "script", strValue))
			scriptfile = strValue;

		if(readXMLString(root, "name", strValue))
			name = strValue;
		else
			name = "";

		if(readXMLInteger(root, "speed", intValue))
			baseSpeed = intValue;
		else
			baseSpeed = 110;

		if(readXMLInteger(root, "attackable", intValue))
			attackable = (intValue != 0);

		if(readXMLInteger(root, "walkinterval", intValue))
			walkTicks = intValue;

		if(readXMLInteger(root, "autowalk", intValue))
		{
			std::cout << "[Notice - Npc::Npc] NPC Name: " << name << " - autowalk has been deprecated, use walkinterval." << std::endl;
			walkTicks = 2000;
		}

		if(readXMLInteger(root, "floorchange", intValue))
			floorChange = (intValue != 0);

		while(p)
		{
			if(xmlStrcmp(p->name, (const xmlChar*)"health") == 0)
			{
				if(readXMLInteger(p, "now", intValue))
					health = intValue;
				else
					health = 100;

				if(readXMLInteger(p, "max", intValue))
					healthMax = intValue;
				else
					healthMax = 100;
			}
			else if(xmlStrcmp(p->name, (const xmlChar*)"look") == 0)
			{
				if(readXMLInteger(p, "type", intValue))
				{
					defaultOutfit.lookType = intValue;

					if(readXMLInteger(p, "head", intValue))
						defaultOutfit.lookHead = intValue;

					if(readXMLInteger(p, "body", intValue))
						defaultOutfit.lookBody = intValue;

					if(readXMLInteger(p, "legs", intValue))
						defaultOutfit.lookLegs = intValue;

					if(readXMLInteger(p, "feet", intValue))
						defaultOutfit.lookFeet = intValue;

					if(readXMLInteger(p, "addons", intValue))
						defaultOutfit.lookAddons = intValue;
				}
				else if(readXMLInteger(p, "typeex", intValue))
					defaultOutfit.lookTypeEx = intValue;

				currentOutfit = defaultOutfit;
			}
			else if(xmlStrcmp(p->name, (const xmlChar*)"parameters") == 0)
			{
				for(xmlNodePtr q = p->children; q != NULL; q = q->next)
				{
					if(xmlStrcmp(q->name, (const xmlChar*)"parameter") == 0)
					{
						std::string paramKey;
						std::string paramValue;
						if(!readXMLString(q, "key", paramKey))
							continue;
						if(!readXMLString(q, "value", paramValue))
							continue;
						m_parameters[paramKey] = paramValue;
					}
				}

			}
			else if(xmlStrcmp(p->name, (const xmlChar*)"interaction") == 0)
			{
				if(readXMLInteger(p, "talkradius", intValue))
					talkRadius = intValue;

				if(readXMLInteger(p, "idletime", intValue))
					idleTime = intValue;

				if(readXMLInteger(p, "idleinterval", intValue))
					idleInterval = intValue;

				if(readXMLInteger(p, "defaultpublic", intValue))
					defaultPublic = intValue != 0;

				responseList = loadInteraction(p->children);
			}
			p = p->next;
		}

		xmlFreeDoc(doc);

		if(!scriptfile.empty())
		{
			m_npcEventHandler = new NpcScript(scriptfile, this);
			if(!m_npcEventHandler->isLoaded())
				return false;
		}
		return true;
	}
	return false;
}

uint32_t Npc::loadParams(xmlNodePtr node)
{
	uint32_t params = RESPOND_DEFAULT;
	std::string strValue;

	if(readXMLString(node, "param", strValue))
	{
		std::vector<std::string> paramList = explodeString(strValue, ";");
		for(std::vector<std::string>::iterator it = paramList.begin(); it != paramList.end(); ++it)
		{
			std::string tmpParam = asLowerCaseString(*it);
			if(tmpParam == "male")
				params |= RESPOND_MALE;
			else if(tmpParam == "female")
				params |= RESPOND_FEMALE;
			else if(tmpParam == "pzblock")
				params |= RESPOND_PZBLOCK;
			else if(tmpParam == "lowmoney")
				params |= RESPOND_LOWMONEY;
			else if(tmpParam == "noamount")
				params |= RESPOND_NOAMOUNT;
			else if(tmpParam == "lowamount")
				params |= RESPOND_LOWAMOUNT;
			else if(tmpParam == "premium")
				params |= RESPOND_PREMIUM;
			else if(tmpParam == "druid")
				params |= RESPOND_DRUID;
			else if(tmpParam == "knight")
				params |= RESPOND_KNIGHT;
			else if(tmpParam == "paladin")
				params |= RESPOND_PALADIN;
			else if(tmpParam == "sorcerer")
				params |= RESPOND_SORCERER;
			else if(tmpParam == "lowlevel")
				params |= RESPOND_LOWLEVEL;
			else
				std::cout << "[Warning - Npc::loadParams] NPC Name: " << name << " - Unknown param " << (*it) << std::endl;
		}
	}
	return params;
}

ResponseList Npc::loadInteraction(xmlNodePtr node)
{
	ResponseList _responseList;
	std::string strValue;
	int32_t intValue;

	while(node)
	{
		if(xmlStrcmp(node->name, (const xmlChar*)"include") == 0)
		{
			if(readXMLString(node, "file", strValue))
			{
				std::string includeFilename = "data/npc/lib/" + strValue;
				xmlDocPtr doc = xmlParseFile(includeFilename.c_str());
				if(doc)
				{
					xmlNodePtr root;
					root = xmlDocGetRootElement(doc);

					if(xmlStrcmp(root->name,(const xmlChar*)"interaction") == 0)
					{
						ResponseList includeResponseList = loadInteraction(root->children);
						_responseList.insert(_responseList.end(), includeResponseList.begin(), includeResponseList.end());
					}
					else
						std::cerr << "Malformed XML" << std::endl;

					xmlFreeDoc(doc);
				}
			}
		}
		else if(xmlStrcmp(node->name, (const xmlChar*)"itemlist") == 0)
		{
			if(readXMLString(node, "listid", strValue))
			{
				ItemListMap::iterator it = itemListMap.find(strValue);
				if(it != itemListMap.end())
				{
					//duplicate listid found
					std::cout << "[Warning - Npc::loadInteraction] NPC Name: " << name << " - Duplicate listId found: " << strValue << std::endl;
				}
				else
				{
					std::string listId = strValue;
					xmlNodePtr tmpNode = node->children;
					std::list<ListItem>& list = itemListMap[strValue];

					while(tmpNode)
					{
						if(xmlStrcmp(tmpNode->name, (const xmlChar*)"item") == 0)
						{
							ListItem li;

							if(readXMLInteger(tmpNode, "id", intValue))
								li.itemId = intValue;
							else
							{
								std::cout << "[Warning - Npc::loadInteraction] NPC Name: " << name << " - Missing list item itemId" << std::endl;
								tmpNode = tmpNode->next;
								continue;
							}

							if(readXMLInteger(tmpNode, "sellprice", intValue))
								li.sellPrice = intValue;

							if(readXMLInteger(tmpNode, "buyprice", intValue))
								li.buyPrice = intValue;

							if(readXMLString(tmpNode, "keywords", strValue))
								li.keywords = strValue;
							else
							{
								std::cout << "[Warning - Npc::loadInteraction] NPC Name: " << name << " - Missing list item keywords" << std::endl;
								tmpNode = tmpNode->next;
								continue;
							}

							//optional
							if(readXMLInteger(tmpNode, "subtype", intValue))
								li.subType = intValue;

							if(readXMLString(tmpNode, "name", strValue))
								li.name = strValue;

							if(readXMLString(tmpNode, "pname", strValue))
								li.pluralName = strValue;

							list.push_back(li);
						}
						tmpNode = tmpNode->next;
					}
				}
			}
		}
		else if(xmlStrcmp(node->name, (const xmlChar*)"interact") == 0)
		{
			NpcResponse::ResponseProperties prop;
			prop.publicize = defaultPublic;

			if(readXMLString(node, "keywords", strValue))
				prop.inputList.push_back(asLowerCaseString(strValue));
			else if(readXMLString(node, "event", strValue))
			{
				strValue = asLowerCaseString(strValue);
				if(strValue == "onbusy")
					hasBusyReply = true;

				prop.interactType = INTERACT_EVENT;
				prop.inputList.push_back(strValue);
			}

			if(readXMLInteger(node, "topic", intValue))
				prop.topic = intValue;

			if(readXMLInteger(node, "focus", intValue))
				prop.focusStatus = intValue;

			if(readXMLInteger(node, "storageId", intValue))
				prop.storageId = intValue;

			if(readXMLInteger(node, "storageValue", intValue))
				prop.storageValue = intValue;

			uint32_t interactParams = loadParams(node);

			if(readXMLString(node, "storageComp", strValue))
			{
				std::string tmpStrValue = asLowerCaseString(strValue);
				if(tmpStrValue == "equal")
					prop.storageComp = STORAGE_EQUAL;
				if(tmpStrValue == "greaterorequal")
					prop.storageComp = STORAGE_GREATEROREQUAL;
				if(tmpStrValue == "greater")
					prop.storageComp = STORAGE_GREATER;
				if(tmpStrValue == "less")
					prop.storageComp = STORAGE_LESS;
				if(tmpStrValue == "lessorequal")
					prop.storageComp = STORAGE_LESSOREQUAL;
			}

			xmlNodePtr tmpNode = node->children;
			while(tmpNode)
			{
				if(xmlStrcmp(tmpNode->name, (const xmlChar*)"keywords") == 0)
				{
					//alternative input keywords
					xmlNodePtr altKeyNode = tmpNode->children;
					while(altKeyNode)
					{
						if(xmlStrcmp(altKeyNode->name, (const xmlChar*)"text") == 0)
						{
							if(readXMLContentString(altKeyNode, strValue))
								prop.inputList.push_back(asLowerCaseString(strValue));
						}
						altKeyNode = altKeyNode->next;
					}
				}
				else if(xmlStrcmp(tmpNode->name, (const xmlChar*)"list") == 0)
				{
					xmlNodePtr listNode = tmpNode->children;
					while(listNode)
					{
						if(xmlStrcmp(listNode->name, (const xmlChar*)"text") == 0)
						{
							if(readXMLContentString(listNode, strValue))
							{
								ItemListMap::iterator it = itemListMap.find(strValue);
								if(it != itemListMap.end())
									prop.itemList.insert(prop.itemList.end(), it->second.begin(), it->second.end());
								else
									std::cout << "[Warning - Npc::loadInteraction] NPC Name: " << name << " - Could not find a list id called: " << strValue << std::endl;
							}
						}
						listNode = listNode->next;
					}
				}
				tmpNode = tmpNode->next;
			}

			tmpNode = node->children;
			while(tmpNode)
			{
				if(xmlStrcmp(tmpNode->name, (const xmlChar*)"response") == 0)
				{
					prop.output = "";
					prop.knowSpell = "";
					prop.params = interactParams | loadParams(tmpNode);
					ScriptVars scriptVars;

					if(readXMLString(tmpNode, "knowspell", strValue))
						prop.knowSpell = strValue;

					if(readXMLString(tmpNode, "text", strValue))
					{
						prop.responseType = RESPONSE_DEFAULT;
						prop.output = strValue;
					}
					else if(readXMLString(tmpNode, "function", strValue))
					{
						prop.responseType = RESPONSE_SCRIPT;
						prop.output = strValue;
					}

					if(readXMLInteger(tmpNode, "public", intValue))
						prop.publicize = (intValue == 1);

					if(readXMLInteger(tmpNode, "b1", intValue))
						scriptVars.b1 = (intValue == 1);

					if(readXMLInteger(tmpNode, "b2", intValue))
						scriptVars.b2 = (intValue == 1);

					if(readXMLInteger(tmpNode, "b3", intValue))
						scriptVars.b3 = (intValue == 1);

					ResponseList subResponseList;

					xmlNodePtr subNode = tmpNode->children;
					while(subNode)
					{
						if(xmlStrcmp(subNode->name, (const xmlChar*)"action") == 0)
						{
							ResponseAction action;

							if(readXMLString(subNode, "name", strValue))
							{
								std::string tmpStrValue = asLowerCaseString(strValue);
								if(tmpStrValue == "topic")
								{
									if(readXMLInteger(subNode, "value", intValue))
									{
										action.actionType = ACTION_SETTOPIC;
										action.intValue = intValue;
									}
								}
								else if(tmpStrValue == "price")
								{
									if(readXMLString(subNode, "value", strValue))
									{
										action.actionType = ACTION_SETPRICE;
										action.strValue = strValue;
										action.intValue = atoi(strValue.c_str());
									}
								}
								else if(tmpStrValue == "amount")
								{
									if(readXMLString(subNode, "value", strValue))
									{
										action.actionType = ACTION_SETAMOUNT;
										action.strValue = strValue;
										action.intValue = atoi(strValue.c_str());
									}
								}
								else if(tmpStrValue == "item")
								{
									if(readXMLInteger(subNode, "value", intValue))
									{
										action.actionType = ACTION_SETITEM;
										action.intValue = intValue;
									}
								}
								else if(tmpStrValue == "subtype")
								{
									if(readXMLInteger(subNode, "value", intValue))
									{
										action.actionType = ACTION_SETSUBTYPE;
										action.intValue = intValue;
									}
								}
								else if(tmpStrValue == "spell")
								{
									if(readXMLString(subNode, "value", strValue))
									{
										action.actionType = ACTION_SETSPELL;
										action.strValue = strValue;

										if(strValue != "|SPELL|")
										{
											InstantSpell* spell = g_spells->getInstantSpellByName(strValue);
											if(!spell)
												std::cout << "[Warning - Npc::loadInteraction] NPC Name: " << name << " - Could not find an instant spell called: " << strValue << std::endl;
										}
									}
								}
								else if(tmpStrValue == "listname")
								{
									if(readXMLString(subNode, "value", strValue))
									{
										action.actionType = ACTION_SETLISTNAME;
										action.strValue = strValue;
									}
								}
								else if(tmpStrValue == "listpname")
								{
									if(readXMLString(subNode, "value", strValue))
									{
										action.actionType = ACTION_SETLISTPNAME;
										action.strValue = strValue;
									}
								}
								else if(tmpStrValue == "teachspell")
								{
									if(readXMLString(subNode, "value", strValue))
									{
										action.actionType = ACTION_TEACHSPELL;
										action.strValue = strValue;

										if(strValue != "|SPELL|")
										{
											InstantSpell* spell = g_spells->getInstantSpellByName(strValue);
											if(!spell)
												std::cout << "[Warning - Npc::loadInteraction] NPC Name: " << name << " - Could not find an instant spell called: " << strValue << std::endl;
										}
									}
								}
								else if(tmpStrValue == "sell")
								{
									if(readXMLString(subNode, "value", strValue))
									{
										action.actionType = ACTION_SELLITEM;
										action.strValue = strValue;
										action.intValue = atoi(strValue.c_str());
									}
								}
								else if(tmpStrValue == "buy")
								{
									if(readXMLString(subNode, "value", strValue))
									{
										action.actionType = ACTION_BUYITEM;
										action.strValue = strValue;
										action.intValue = atoi(strValue.c_str());
									}
								}
								else if(tmpStrValue == "takemoney")
								{
									if(readXMLString(subNode, "value", strValue))
									{
										action.actionType = ACTION_TAKEMONEY;
										action.strValue = strValue;
										action.intValue = atoi(strValue.c_str());
									}
								}
								else if(tmpStrValue == "givemoney")
								{
									if(readXMLString(subNode, "value", strValue))
									{
										action.actionType = ACTION_GIVEMONEY;
										action.strValue = strValue;
										action.intValue = atoi(strValue.c_str());
									}
								}
								else if(tmpStrValue == "level")
								{
									if(readXMLString(subNode, "value", strValue))
									{
										action.actionType = ACTION_SETLEVEL;
										action.strValue = strValue;
										action.intValue = atoi(strValue.c_str());
									}
								}
								else if(tmpStrValue == "giveitem")
								{
									if(readXMLString(subNode, "value", strValue))
									{
										action.actionType = ACTION_GIVEITEM;
										action.strValue = strValue;
										action.intValue = atoi(strValue.c_str());
									}
								}
								else if(tmpStrValue == "takeitem")
								{
									if(readXMLString(subNode, "value", strValue))
									{
										action.actionType = ACTION_TAKEITEM;
										action.strValue = strValue;
										action.intValue = atoi(strValue.c_str());
									}
								}
								else if(tmpStrValue == "effect")
								{
									if(readXMLString(subNode, "value", strValue))
									{
										action.actionType = ACTION_SETEFFECT;
										action.intValue = getMagicEffect(strValue);
									}
								}
								else if(tmpStrValue == "idle")
								{
									if(readXMLInteger(subNode, "value", intValue))
									{
										action.actionType = ACTION_SETIDLE;
										action.intValue = intValue;
									}
								}
								else if(tmpStrValue == "script")
								{
									if(readXMLString(subNode, "value", strValue))
									{
										action.actionType = ACTION_SCRIPT;
										action.strValue = strValue;
									}
									else
									{
										xmlNodePtr scriptNode = subNode->children;
										while(scriptNode)
										{
											if(xmlStrcmp(scriptNode->name, (const xmlChar*)"text") == 0 ||
												scriptNode->type == XML_CDATA_SECTION_NODE)
											{
												if(readXMLContentString(scriptNode, strValue))
												{
													trim_left(strValue, "\r");
													trim_left(strValue, "\n");
													trim_left(strValue, " ");
													if(strValue.length() > action.strValue.length())
													{
														action.actionType = ACTION_SCRIPT;
														action.strValue = strValue;
													}
												}
											}
											scriptNode = scriptNode->next;
										}
									}
								}
								else if(tmpStrValue == "scriptparam")
								{
									if(readXMLString(subNode, "value", strValue))
									{
										action.actionType = ACTION_SCRIPTPARAM;
										action.strValue = strValue;
									}
								}
								else if(tmpStrValue == "storage")
								{
									if(readXMLInteger(subNode, "value", intValue))
									{
										action.actionType = ACTION_SETSTORAGE;
										action.intValue = intValue;
									}
								}
								else if(tmpStrValue == "addqueue")
								{
									if(readXMLString(subNode, "value", strValue))
									{
										action.actionType = ACTION_ADDQUEUE;
										action.strValue = strValue;
										action.intValue = atoi(strValue.c_str());
									}
								}
								else if(tmpStrValue == "teleport")
								{
									if(readXMLString(subNode, "value", strValue))
									{
										std::vector<std::string> posList = explodeString(strValue, ";");
										action.actionType = ACTION_SETTELEPORT;
										action.strValue = strValue;
										action.pos.x = 0;
										action.pos.y = 0;
										action.pos.z = 0;

										if(posList.size() == 3)
										{
											action.pos.x = atoi(posList[0].c_str());
											action.pos.y = atoi(posList[1].c_str());
											action.pos.z = atoi(posList[2].c_str());
										}
									}
								}
								else
									std::cout << "[Warning - Npc::loadInteraction] Unknown action " << strValue << std::endl;
							}

							if(readXMLInteger(subNode, "key", intValue))
								action.key = intValue;

							if(action.actionType != ACTION_NONE)
								prop.actionList.push_back(action);
						}
						else if(xmlStrcmp(subNode->name, (const xmlChar*)"interact") == 0)
						{
							if(subResponseList.empty())
							{
								ResponseList nodeResponseList = loadInteraction(subNode);
								subResponseList.insert(subResponseList.end(),
									nodeResponseList.begin(), nodeResponseList.end());
							}
						}
						subNode = subNode->next;
					}

					//Check if this interaction has a |list| keyword
					bool hasListKeyword = false;
					for(std::list<std::string>::iterator it = prop.inputList.begin();
						it != prop.inputList.end(); ++it)
					{
						if((*it).find("|list|") != std::string::npos)
						{
							hasListKeyword = true;
							break;
						}
					}

					//Iterate through all input keywords and replace all |LIST| with the item list
					if(hasListKeyword && !prop.itemList.empty())
					{
						for(std::list<ListItem>::iterator it = prop.itemList.begin(); it != prop.itemList.end(); ++it)
						{
							NpcResponse::ResponseProperties listItemProp = prop;
							for(std::list<std::string>::iterator iit = listItemProp.inputList.begin();
								iit != listItemProp.inputList.end(); ++iit)
							{
								std::string& input = (*iit);

								if(input.find("|list|") == std::string::npos)
									continue;

								//Replace |LIST| with the keyword in the list
								replaceString(input, "|list|", (*it).keywords);

								ResponseAction action;

								action.actionType = ACTION_SETITEM;
								action.intValue = (*it).itemId;
								listItemProp.actionList.push_front(action);

								action.actionType = ACTION_SETSELLPRICE;
								action.intValue = (*it).sellPrice;
								listItemProp.actionList.push_front(action);

								action.actionType = ACTION_SETBUYPRICE;
								action.intValue = (*it).buyPrice;
								listItemProp.actionList.push_front(action);

								action.actionType = ACTION_SETSUBTYPE;
								action.intValue = (*it).subType;
								listItemProp.actionList.push_front(action);

								action.actionType = ACTION_SETSUBTYPE;
								action.intValue = (*it).subType;
								listItemProp.actionList.push_front(action);

								action.actionType = ACTION_SETLISTNAME;
								if(!(*it).name.empty())
									action.strValue = (*it).name;
								else
								{
									const ItemType& itemType = Item::items[(*it).itemId];
									if(itemType.id != 0)
										action.strValue = itemType.article + " " + itemType.name;
								}
								listItemProp.actionList.push_front(action);

								action.actionType = ACTION_SETLISTPNAME;
								if(!(*it).pluralName.empty())
									action.strValue = (*it).pluralName;
								else
								{
									const ItemType& itemType = Item::items[(*it).itemId];
									if(itemType.id != 0)
										action.strValue = itemType.pluralName;
								}
								listItemProp.actionList.push_front(action);

								ResponseList list;
								for(ResponseList::iterator respIter = subResponseList.begin();
									respIter != subResponseList.end(); ++respIter)
								{
										list.push_back(new NpcResponse(*(*respIter)));
								}

								//Create a new response for this list item
								NpcResponse* response = new NpcResponse(listItemProp, list, scriptVars);
								_responseList.push_back(response);
							}
						}
					}
					else
					{
						NpcResponse* response = new NpcResponse(prop, subResponseList, scriptVars);
						_responseList.push_back(response);
					}
				}
				tmpNode = tmpNode->next;
			}
		}
		node = node->next;
	}
	return _responseList;
}

NpcState* Npc::getState(const Player* player, bool makeNew /*= true*/)
{
	for(StateList::iterator it = stateList.begin(); it != stateList.end(); ++it)
	{
		if((*it)->respondToCreature == player->getID())
			return *it;
	}

	if(!makeNew)
		return NULL;

	NpcState* state = new NpcState;
	state->prevInteraction = 0;
	state->price = 0;
	state->sellPrice = 0;
	state->buyPrice = 0;
	state->amount = 1;
	state->itemId = 0;
	state->subType = -1;
	state->spellName = "";
	state->listName = "";
	state->listPluralName = "";
	state->level = -1;
	state->topic = -1;
	state->isIdle = true;
	state->isQueued = false;
	state->respondToText = "";
	state->respondToCreature = 0;
	state->lastResponse = NULL;
	state->prevRespondToText = "";
	stateList.push_back(state);
	return state;
}

bool Npc::canSee(const Position& pos) const
{
	if(pos.z != getPosition().z)
		return false;

	return Creature::canSee(getPosition(), pos, Map::maxClientViewportX, Map::maxClientViewportY);
}

std::string Npc::getDescription(int32_t lookDistance) const
{
	std::stringstream s;
	s << name << ".";
	return s.str();
}

void Npc::onAddTileItem(const Tile* tile, const Position& pos, const Item* item)
{
	Creature::onAddTileItem(tile, pos, item);
}

void Npc::onUpdateTileItem(const Tile* tile, const Position& pos, uint32_t stackpos,
	const Item* oldItem, const ItemType& oldType, const Item* newItem, const ItemType& newType)
{
	Creature::onUpdateTileItem(tile, pos, stackpos, oldItem, oldType, newItem, newType);
}

void Npc::onRemoveTileItem(const Tile* tile, const Position& pos, uint32_t stackpos,
	const ItemType& iType, const Item* item)
{
	Creature::onRemoveTileItem(tile, pos, stackpos, iType, item);
}

void Npc::onUpdateTile(const Tile* tile, const Position& pos)
{
	Creature::onUpdateTile(tile, pos);
}

void Npc::onCreatureAppear(const Creature* creature, bool isLogin)
{
	Creature::onCreatureAppear(creature, isLogin);

	if(creature == this && walkTicks > 0)
		addEventWalk();

	if(creature == this)
	{
		if(m_npcEventHandler)
			m_npcEventHandler->onCreatureAppear(creature);
	}
	//only players for script events
	else if(Player* player = const_cast<Player*>(creature->getPlayer()))
	{
		if(m_npcEventHandler)
			m_npcEventHandler->onCreatureAppear(creature);

		NpcState* npcState = getState(player);
		if(npcState)
		{
			if(canSee(player->getPosition()))
			{
				npcState->respondToCreature = player->getID();
				onPlayerEnter(player, npcState);
			}
		}
	}
}

void Npc::onCreatureDisappear(const Creature* creature, uint32_t stackpos, bool isLogout)
{
	Creature::onCreatureDisappear(creature, stackpos, isLogout);

	if(creature == this)
	{
		//Close all open shop window's
		closeAllShopWindows();
	}
	else if(Player* player = const_cast<Player*>(creature->getPlayer()))
	{
		if(m_npcEventHandler)
			m_npcEventHandler->onCreatureDisappear(creature);

		NpcState* npcState = getState(player);
		if(npcState)
		{
			npcState->respondToCreature = player->getID();
			onPlayerLeave(player, npcState);
		}
	}
}

void Npc::onCreatureMove(const Creature* creature, const Tile* newTile, const Position& newPos,
		const Tile* oldTile, const Position& oldPos, uint32_t oldStackPos, bool teleport)
{
	Creature::onCreatureMove(creature, newTile, newPos, oldTile, oldPos, oldStackPos, teleport);

	if(creature == this)
	{
		if(m_npcEventHandler)
			m_npcEventHandler->onCreatureMove(creature, oldPos, newPos);
	}
	else if(Player* player = const_cast<Player*>(creature->getPlayer()))
	{
		if(m_npcEventHandler)
			m_npcEventHandler->onCreatureMove(creature, oldPos, newPos);

		NpcState* npcState = getState(player);
		if(npcState)
		{
			bool canSeeNewPos = canSee(newPos);
			bool canSeeOldPos = canSee(oldPos);

			if(canSeeNewPos && !canSeeOldPos)
			{
				npcState->respondToCreature = player->getID();
				onPlayerEnter(player, npcState);
			}
			else if(!canSeeNewPos && canSeeOldPos)
			{
				npcState->respondToCreature = player->getID();
				onPlayerLeave(player, npcState);
			}
			else if(canSeeNewPos && canSeeOldPos)
			{
				npcState->respondToCreature = player->getID();
				const NpcResponse* response = getResponse(player, npcState, EVENT_PLAYER_MOVE);
				executeResponse(player, npcState, response);
			}
		}
	}
}

void Npc::onCreatureTurn(const Creature* creature, uint32_t stackpos)
{
	Creature::onCreatureTurn(creature, stackpos);
}

void Npc::onCreatureSay(const Creature* creature, SpeakClasses type, const std::string& text)
{
	if(creature->getID() == this->getID())
		return;

	//only players for script events
	if(const Player* player = creature->getPlayer())
	{
		if(m_npcEventHandler)
			m_npcEventHandler->onCreatureSay(player, type, text);

		if(type == SPEAK_SAY || type == SPEAK_PRIVATE_PN)
		{
			const Position& myPos = getPosition();
			const Position& pos = creature->getPosition();
			if(canSee(myPos))
			{
				if ((pos.x >= myPos.x - talkRadius) && (pos.x <= myPos.x + talkRadius) &&
					(pos.y >= myPos.y - talkRadius) && (pos.y <= myPos.y + talkRadius))
				{
					NpcState* npcState = getState(player);
					npcState->respondToText = text;
					npcState->respondToCreature = player->getID();
				}
			}
		}
	}
}

void Npc::onPlayerCloseChannel(const Player* player)
{
	if(m_npcEventHandler)
		m_npcEventHandler->onPlayerCloseChannel(player);
}

void Npc::onCreatureChangeOutfit(const Creature* creature, const Outfit_t& outfit)
{
	#ifdef __DEBUG_NPC__
	std::cout << "Npc::onCreatureChangeOutfit" << std::endl;
	#endif
}

void Npc::onPlayerEnter(Player* player, NpcState* state)
{
	const NpcResponse* response = getResponse(player, state, EVENT_PLAYER_ENTER);
	executeResponse(player, state, response);
}

void Npc::onPlayerLeave(Player* player, NpcState* state)
{
	player->closeShopWindow();
	const NpcResponse* response = getResponse(player, state, EVENT_PLAYER_LEAVE);
	executeResponse(player, state, response);
}

void Npc::onThink(uint32_t interval)
{
	Creature::onThink(interval);
	if(m_npcEventHandler)
		m_npcEventHandler->onThink();

	isIdle = true;
	bool idleResponse = false;
	#define MAX_RAND_RANGE 10000000
	if(((uint32_t)MAX_RAND_RANGE * (EVENT_CREATURE_THINK_INTERVAL / 1000)) / idleInterval >= (uint32_t)random_range(0, MAX_RAND_RANGE))
		idleResponse = true;

	for(StateList::iterator it = stateList.begin(); it != stateList.end();)
	{
		NpcState* npcState = *it;
		const NpcResponse* response = NULL;

		Player* player = g_game.getPlayerByID(npcState->respondToCreature);
		bool closeConversation = false;
		bool idleTimeout = false;
		if(!npcState->isQueued)
		{
			if(npcState->prevInteraction == 0)
				npcState->prevInteraction = OTSYS_TIME();

			if(idleTime > 0 && (OTSYS_TIME() - npcState->prevInteraction) > (uint64_t)(idleTime * 1000))
			{
				idleTimeout = true;
				closeConversation = true;
			}
		}

		if(idleResponse && player)
		{
			response = getResponse(player, EVENT_IDLE);
			executeResponse(player, npcState, response);
			idleResponse = false;
		}

		if(!player || closeConversation)
		{
			if(queueList.empty())
			{
				if(idleTimeout && player)
					onPlayerLeave(player, npcState);
			}
			else
			{
				Player* nextPlayer = NULL;
				while(!queueList.empty())
				{
					nextPlayer = g_game.getPlayerByID(*queueList.begin());
					if(nextPlayer)
					{
						NpcState* nextPlayerState = getState(nextPlayer, false);
						if(nextPlayerState)
						{
							nextPlayerState->respondToText = nextPlayerState->prevRespondToText;
							nextPlayerState->isQueued = false;
							break;
						}
					}
					queueList.erase(queueList.begin());
				}
			}

			delete *it;
			stateList.erase(it++);
			//std::cout << "Closing conversation." << std::endl;
			continue;
		}

		if(!npcState->respondToText.empty())
		{
			if(hasBusyReply && !isIdle)
			{
				//Check if we have a busy reply
				response = getResponse(player, npcState, EVENT_BUSY);
				if(response)
					executeResponse(player, npcState, response);
			}
			else
			{
				if(npcState->lastResponse)
				{
					//Check previous response chain first
					const ResponseList& list = npcState->lastResponse->getResponseList();
					response = getResponse(list, player, npcState, npcState->respondToText);
				}

				if(!response)
					response = getResponse(player, npcState, npcState->respondToText);

				if(response)
				{
					setCreatureFocus(player);
					executeResponse(player, npcState, response);
				}
			}

			npcState->prevRespondToText = npcState->respondToText;
			npcState->respondToText = "";
		}

		response = getResponse(player, npcState, EVENT_THINK);
		executeResponse(player, npcState, response);

		if(!npcState->isIdle)
		{
			isIdle = false;

			if(hasBusyReply)
				setCreatureFocus(player);
		}
		++it;
	}

	if(isIdle && !hasScriptedFocus)
		setCreatureFocus(NULL);
}

void Npc::executeResponse(Player* player, NpcState* npcState, const NpcResponse* response)
{
	if(response)
	{
		npcState->lastResponse = response;
		bool resetTopic = true;

		if(response->getFocusState() == 0)
			npcState->isIdle = true;
		else if(response->getFocusState() == 1)
			npcState->isIdle = false;

		if(response->getAmount() != -1)
			npcState->amount = response->getAmount();

		for(ActionList::const_iterator it = response->getFirstAction(); it != response->getEndAction(); ++it)
		{
			switch((*it).actionType)
			{
				case ACTION_SETTOPIC: npcState->topic = (*it).intValue; resetTopic = false; break;
				case ACTION_SETSELLPRICE: npcState->sellPrice = (*it).intValue; break;
				case ACTION_SETBUYPRICE: npcState->buyPrice = (*it).intValue; break;
				case ACTION_SETITEM: npcState->itemId = (*it).intValue; break;
				case ACTION_SETSUBTYPE: npcState->subType = (*it).intValue; break;
				case ACTION_SETEFFECT: g_game.addMagicEffect(player->getPosition(), (*it).intValue); break;
				case ACTION_SETPRICE:
				{
					if((*it).strValue == "|SELLPRICE|")
						npcState->price = npcState->sellPrice;
					else if((*it).strValue == "|BUYPRICE|")
						npcState->price = npcState->buyPrice;
					else
						npcState->price = (*it).intValue;
					break;
				}

				case ACTION_SETTELEPORT:
				{
					Position teleportTo = (*it).pos;
					if((*it).strValue == "|TEMPLE|")
						teleportTo = player->getTemplePosition();
					g_game.internalTeleport(player, teleportTo, true);
					break;
				}

				case ACTION_SETIDLE:
				{
					npcState->isIdle = ((*it).intValue == 1);
					break;
				}

				case ACTION_SETLEVEL:
				{
					if((*it).strValue == "|SPELLLEVEL|")
					{
						npcState->level = -1;
						InstantSpell* spell = g_spells->getInstantSpellByName(npcState->spellName);
						if(spell)
							npcState->level = spell->getLevel();
					}
					else
						npcState->level = (*it).intValue;
					break;
				}

				case ACTION_SETSPELL:
				{
					npcState->spellName = "";
					InstantSpell* spell = g_spells->getInstantSpellByName((*it).strValue);
					if(spell)
						npcState->spellName = (*it).strValue;
					break;
				}

				case ACTION_SETLISTNAME:
				{
					npcState->listName = (*it).strValue;
					break;
				}

				case ACTION_SETLISTPNAME:
				{
					npcState->listPluralName = (*it).strValue;
					break;
				}

				case ACTION_SETAMOUNT:
				{
					int32_t amount = 1;
					if((*it).strValue == "|AMOUNT|")
						amount = npcState->amount;
					else
						amount = (*it).intValue;

					npcState->amount = amount;
					break;
				}

				case ACTION_TEACHSPELL:
				{
					std::string spellName = "";
					if((*it).strValue == "|SPELL|")
						spellName = npcState->spellName;
					else
						spellName = (*it).strValue;

					player->learnInstantSpell(spellName);
					break;
				}

				case ACTION_SETSTORAGE:
				{
					if((*it).key > 0)
						player->addStorageValue((*it).key, (*it).intValue);
					break;
				}

				case ACTION_ADDQUEUE:
				{
					QueueList::iterator it = std::find(queueList.begin(), queueList.end(), player->getID());
					if(it == queueList.end())
					{
						queueList.push_back(player->getID());
						npcState->isQueued = true;
					}
					break;
				}

				case ACTION_SELLITEM:
				{
					uint32_t moneyCount = 0;
					if((*it).strValue == "|PRICE|")
						moneyCount = npcState->price * npcState->amount;
					else
						moneyCount = (*it).intValue;

					const ItemType& it = Item::items[npcState->itemId];
					if(it.id != 0)
					{
						int32_t subType = -1;
						if(it.hasSubType())
							subType = npcState->subType;

						int32_t itemCount = player->__getItemTypeCount(it.id, subType);
						if(itemCount >= npcState->amount)
						{
							g_game.removeItemOfType(player, it.id, npcState->amount, subType);
							g_game.addMoney(player, moneyCount, FLAG_NOLIMIT);
						}
					}
					break;
				}

				case ACTION_BUYITEM:
				{
					uint32_t moneyCount = 0;
					if((*it).strValue == "|PRICE|")
						moneyCount = npcState->price * npcState->amount;
					else
						moneyCount = (*it).intValue;

					const ItemType& it = Item::items[npcState->itemId];
					if(it.id != 0)
					{
						int32_t subType = -1;
						if(it.hasSubType())
							subType = npcState->subType;

						if(g_game.removeMoney(player, moneyCount))
						{
							for(int32_t i = 0; i < npcState->amount; ++i)
							{
								Item* item = Item::CreateItem(it.id, subType);
								if(g_game.internalPlayerAddItem(player, item) != RET_NOERROR)
									delete item;
							}
						}
						else
							std::cout << "Error [Npc::executeResponse] Not enough money: " << player->getName() << "\tNpc: " << getName() << std::endl;
					}
					break;
				}

				case ACTION_TAKEITEM:
				{
					int32_t itemId = 0;
					if((*it).strValue == "|ITEM|")
						itemId = npcState->itemId;
					else
						itemId = (*it).intValue;

					const ItemType& it = Item::items[npcState->itemId];
					if(it.id != 0)
					{
						int32_t subType = -1;
						if(it.hasSubType())
							subType = npcState->subType;

						int32_t itemCount = player->__getItemTypeCount(itemId, subType);
						if(itemCount >= npcState->amount)
							g_game.removeItemOfType(player, itemId, npcState->amount, subType);
					}
					break;
				}

				case ACTION_GIVEITEM:
				{
					int32_t itemId = 0;
					if((*it).strValue == "|ITEM|")
						itemId = npcState->itemId;
					else
						itemId = (*it).intValue;

					const ItemType& it = Item::items[itemId];
					if(it.id != 0)
					{
						int32_t subType = -1;
						if(it.hasSubType())
							subType = npcState->subType;

						for(int32_t i = 0; i < npcState->amount; ++i)
						{
							Item* item = Item::CreateItem(it.id, subType);
							if(g_game.internalPlayerAddItem(player, item) != RET_NOERROR)
								delete item;
						}
					}
					break;
				}

				case ACTION_TAKEMONEY:
				{
					uint32_t moneyCount = 0;
					if((*it).strValue == "|PRICE|")
						moneyCount = npcState->price * npcState->amount;
					else
						moneyCount = (*it).intValue;

					g_game.removeMoney(player, moneyCount);
					break;
				}

				case ACTION_GIVEMONEY:
				{
					uint32_t moneyCount = 0;
					if((*it).strValue == "|PRICE|")
						moneyCount = npcState->price * npcState->amount;
					else
						moneyCount = (*it).intValue;

					g_game.addMoney(player, moneyCount);
					break;
				}

				case ACTION_SCRIPT:
				{
					NpcScriptInterface scriptInterface;
					if(scriptInterface.reserveScriptEnv())
					{
						ScriptEnviroment* env = m_scriptInterface->getScriptEnv();

						std::stringstream scriptstream;
						//attach various variables that could be interesting
						scriptstream << "cid = " << env->addThing(player) << std::endl;
						scriptstream << "text = \"" << npcState->respondToText << "\"" << std::endl;
						scriptstream << "name = \"" << player->getName() << "\"" << std::endl;
						scriptstream << "idletime = " << idleTime << std::endl;
						scriptstream << "idleinterval = " << idleInterval << std::endl;

						scriptstream << "itemlist = {" << std::endl;
						uint32_t n = 0;
						for(std::list<ListItem>::const_iterator iit = response->prop.itemList.begin(); iit != response->prop.itemList.end(); ++iit)
						{
							bool adddelim = (n + 1 != response->prop.itemList.size());
							scriptstream << "{id = " << iit->itemId
								<< ", subtype = " << iit->subType
								<< ", buy=" << iit->buyPrice
								<< ", sell=" << iit->sellPrice << "}";

							if(adddelim)
								scriptstream << "," << std::endl;
							++n;
						}
						scriptstream << "}" << std::endl;

						scriptstream << "_state = {" << std::endl;
						scriptstream << "topic = " << npcState->topic << ',' << std::endl;
						scriptstream << "itemid = " << npcState->itemId << ',' << std::endl;
						scriptstream << "subtype = " << npcState->subType << ',' << std::endl;
						scriptstream << "amount = " << npcState->amount << ',' << std::endl;
						scriptstream << "price = " << npcState->price << ',' << std::endl;
						scriptstream << "level = " << npcState->level << ',' << std::endl;
						scriptstream << "spellname = \"" << npcState->spellName << "\"" << ',' << std::endl;
						scriptstream << "listname = \"" << npcState->listName << "\"" << ',' << std::endl;
						scriptstream << "listpname = \"" << npcState->listPluralName << "\"" << ',' << std::endl;

						scriptstream << "n1 = " << npcState->scriptVars.n1 << ',' << std::endl;
						scriptstream << "n2 = " << npcState->scriptVars.n2 << ',' << std::endl;
						scriptstream << "n3 = " << npcState->scriptVars.n3 << ',' << std::endl;

						scriptstream << "b1 = " << (npcState->scriptVars.b1 ? "true" : "false" ) << ',' << std::endl;
						scriptstream << "b2 = " << (npcState->scriptVars.b2 ? "true" : "false" ) << ',' << std::endl;
						scriptstream << "b3 = " << (npcState->scriptVars.b3 ? "true" : "false" ) << ',' << std::endl;

						scriptstream << "s1 = \"" << npcState->scriptVars.s1 << "\"" << ',' << std::endl;
						scriptstream << "s2 = \"" << npcState->scriptVars.s2 << "\"" << ',' << std::endl;
						scriptstream << "s3 = \"" << npcState->scriptVars.s3 << "\"" << std::endl;
						scriptstream << "}" << std::endl;

						scriptstream << (*it).strValue;

						//std::cout << scriptstream.str() << std::endl;

						if(scriptInterface.loadBuffer(scriptstream.str(), this) != -1)
						{
							lua_State* L = scriptInterface.getLuaState();
							lua_getglobal(L, "_state");
							NpcScriptInterface::popState(L, npcState);
						}
						scriptInterface.releaseScriptEnv();
					}
					break;
				}
				default: break;
			}
		}

		if(response->getResponseType() == RESPONSE_DEFAULT)
		{
			std::string responseString = formatResponse(player, npcState, response);
			if(!responseString.empty())
			{
				if(response->publicize())
					g_game.internalCreatureSay(this, SPEAK_SAY, responseString);
				else
					g_game.npcSpeakToPlayer(this, player, responseString, false);
			}
		}
		else
		{
			int32_t functionId = -1;
			ResponseScriptMap::iterator it = responseScriptMap.find(response->getText());
			if(it != responseScriptMap.end())
				functionId = it->second;
			else
			{
				functionId = m_scriptInterface->getEvent(response->getText());
				responseScriptMap[response->getText()] = functionId;
			}

			if(functionId != -1)
			{
				if(m_scriptInterface->reserveScriptEnv())
				{
					ScriptEnviroment* env = m_scriptInterface->getScriptEnv();

					lua_State* L = m_scriptInterface->getLuaState();

					env->setScriptId(functionId, m_scriptInterface);
					Npc* prevNpc = env->getNpc();
					env->setRealPos(getPosition());
					env->setNpc(this);

					m_scriptInterface->pushFunction(functionId);
					int32_t paramCount = 0;
					for(ActionList::const_iterator it = response->getFirstAction(); it != response->getEndAction(); ++it)
					{
						if((*it).actionType == ACTION_SCRIPTPARAM)
						{
							if((*it).strValue == "|PLAYER|")
							{
								uint32_t cid = env->addThing(player);
								lua_pushnumber(L, cid);
							}
							else if((*it).strValue == "|TEXT|")
								lua_pushstring(L, npcState->respondToText.c_str());
							else
							{
								std::cout << "[Warning - Npc::executeResponse] Unknown script param: " << (*it).strValue << std::endl;
								break;
							}
							++paramCount;
						}
					}

					NpcScriptInterface::pushState(L, npcState);
					lua_setglobal(L, "_state");
					m_scriptInterface->callFunction(paramCount);
					lua_getglobal(L, "_state");
					NpcScriptInterface::popState(L, npcState);
					if(prevNpc)
					{
						env->setRealPos(prevNpc->getPosition());
						env->setNpc(prevNpc);
					}
					m_scriptInterface->releaseScriptEnv();
				}
				else
					std::cout << "[Error] Call stack overflow." << std::endl;
			}
		}

		if(resetTopic && response->getTopic() == npcState->topic)
			npcState->topic = -1;

		npcState->prevInteraction = OTSYS_TIME();
	}
}

void Npc::doSay(std::string msg, Player* focus /*= NULL*/, bool publicize /*= false*/)
{
	g_game.npcSpeakToPlayer(this, focus, msg, publicize);
}

void Npc::doMove(Direction dir)
{
	g_game.internalMoveCreature(this, dir);
}

void Npc::doTurn(Direction dir)
{
	g_game.internalCreatureTurn(this, dir);
}

uint32_t Npc::getListItemPrice(uint16_t itemId, ShopEvent_t type)
{
	for(ItemListMap::iterator it = itemListMap.begin(); it != itemListMap.end(); ++it)
	{
		std::list<ListItem>& itemList = it->second;
		for(std::list<ListItem>::iterator iit = itemList.begin(); iit != itemList.end(); ++iit)
		{
			if((*iit).itemId == itemId)
			{
				if(type == SHOPEVENT_BUY)
					return (*iit).buyPrice;
				else if(type == SHOPEVENT_SELL)
					return (*iit).sellPrice;
			}
		}
	}
	return 0;
}

void Npc::onPlayerTrade(Player* player, ShopEvent_t type, int32_t callback, uint16_t itemId,
	uint8_t count, uint8_t amount)
{
	if(type == SHOPEVENT_BUY)
	{
		NpcState* npcState = getState(player, true);
		if(npcState)
		{
			npcState->amount = amount;
			npcState->subType = count;
			npcState->itemId = itemId;
			npcState->buyPrice = getListItemPrice(itemId, SHOPEVENT_BUY);
			const NpcResponse* response = getResponse(player, npcState, EVENT_PLAYER_SHOPBUY);
			executeResponse(player, npcState, response);
		}
	}
	else if(type == SHOPEVENT_SELL)
	{
		NpcState* npcState = getState(player, true);
		if(npcState)
		{
			npcState->amount = amount;
			npcState->subType = count;
			npcState->itemId = itemId;
			npcState->sellPrice = getListItemPrice(itemId, SHOPEVENT_SELL);
			const NpcResponse* response = getResponse(player, npcState, EVENT_PLAYER_SHOPSELL);
			executeResponse(player, npcState, response);
		}
	}

	if(m_npcEventHandler)
		m_npcEventHandler->onPlayerTrade(player, callback, itemId, count, amount);

	player->sendCash(g_game.getMoney(const_cast<Player*>(player)));
}

void Npc::onPlayerEndTrade(Player* player, int32_t buyCallback,
	int32_t sellCallback)
{
	lua_State* L = getScriptInterface()->getLuaState();
	if(buyCallback != -1)
		luaL_unref(L, LUA_REGISTRYINDEX, buyCallback);
	if(sellCallback != -1)
		luaL_unref(L, LUA_REGISTRYINDEX, sellCallback);

	removeShopPlayer(player);

	NpcState* npcState = getState(player, true);
	if(npcState)
	{
		const NpcResponse* response = getResponse(player, npcState, EVENT_PLAYER_SHOPCLOSE);
		executeResponse(player, npcState, response);
	}

	if(m_npcEventHandler)
		m_npcEventHandler->onPlayerEndTrade(player);
}

bool Npc::getNextStep(Direction& dir)
{
	if(Creature::getNextStep(dir))
		return true;

	if(walkTicks <= 0)
		return false;

	if(!isIdle || focusCreature != 0)
		return false;

	if(getTimeSinceLastMove() < walkTicks)
		return false;

	return getRandomStep(dir);
}

bool Npc::canWalkTo(const Position& fromPos, Direction dir)
{
	Position toPos = fromPos;
	toPos = getNextPosition(dir, toPos);

	bool result = Spawns::getInstance()->isInZone(masterPos, masterRadius, toPos);
	if(!result)
		return false;

	Tile* tile = g_game.getTile(toPos.x, toPos.y, toPos.z);
	if(!tile || tile->__queryAdd(0, this, 1, 0) != RET_NOERROR)
		return false;

	if(!floorChange && (tile->floorChange() || tile->getTeleportItem()))
		return false;

	return true;
}

bool Npc::getRandomStep(Direction& dir)
{
	std::vector<Direction> dirList;
	const Position& creaturePos = getPosition();

	if(canWalkTo(creaturePos, NORTH))
		dirList.push_back(NORTH);

	if(canWalkTo(creaturePos, SOUTH))
		dirList.push_back(SOUTH);

	if(canWalkTo(creaturePos, EAST))
		dirList.push_back(EAST);

	if(canWalkTo(creaturePos, WEST))
		dirList.push_back(WEST);

	if(!dirList.empty())
	{
		std::random_shuffle(dirList.begin(), dirList.end());
		dir = dirList[random_range(0, dirList.size() - 1)];
		return true;
	}
	return false;
}

void Npc::doMoveTo(Position target)
{
	std::list<Direction> listDir;
	if(!g_game.getPathToEx(this, target, listDir, 1, 1, true, true))
		return;

	startAutoWalk(listDir);
}

void Npc::setCreatureFocus(Creature* creature)
{
	if(creature)
	{
		focusCreature = creature->getID();
		const Position& creaturePos = creature->getPosition();
		const Position& myPos = getPosition();
		int32_t dx = myPos.x - creaturePos.x;
		int32_t dy = myPos.y - creaturePos.y;

		Direction dir = SOUTH;
		float tan = 0;

		if(dx != 0)
			tan = dy/dx;
		else
			tan = 10;

		if(std::abs(tan) < 1)
		{
			if(dx > 0)
				dir = WEST;
			else
				dir = EAST;
		}
		else
		{
			if(dy > 0)
				dir = NORTH;
			else
				dir = SOUTH;
		}
		g_game.internalCreatureTurn(this, dir);
	}
	else
		focusCreature = 0;
}

const NpcResponse* Npc::getResponse(const ResponseList& list, const Player* player,
	NpcState* npcState, const std::string& text, bool exactMatch /*= false*/)
{
	std::string textString = asLowerCaseString(text);
	std::vector<std::string> wordList = explodeString(textString, " ");
	NpcResponse* response = NULL;
	int32_t bestMatchCount = 0;
	int32_t totalMatchCount = 0;

	for(ResponseList::const_iterator it = list.begin(); it != list.end(); ++it)
	{
		int32_t matchCount = 0;

		if((*it)->getParams() != RESPOND_DEFAULT)
		{
			uint32_t params = (*it)->getParams();
			if(hasBitSet(RESPOND_MALE, params))
			{
				if(!player->getSex() == PLAYERSEX_MALE)
					continue;
				++matchCount;
			}

			if(hasBitSet(RESPOND_FEMALE, params))
			{
				if(!player->getSex() == PLAYERSEX_FEMALE)
					continue;
				++matchCount;
			}

			if(hasBitSet(RESPOND_PZBLOCK, params))
			{
				if(!player->isPzLocked())
					continue;
				++matchCount;
			}

			if(hasBitSet(RESPOND_PREMIUM, params))
			{
				if(!player->isPremium())
					continue;
				++matchCount;
			}

			if(hasBitSet(RESPOND_DRUID, params))
			{
				if(player->getVocationId() != 2)
					continue;
				++matchCount;
			}

			if(hasBitSet(RESPOND_KNIGHT, params))
			{
				if(player->getVocationId() != 4)
					continue;
				++matchCount;
			}

			if(hasBitSet(RESPOND_PALADIN, params))
			{
				if(player->getVocationId() != 3)
					continue;
				++matchCount;
			}

			if(hasBitSet(RESPOND_SORCERER, params))
			{
				if(player->getVocationId() != 1)
					continue;
				++matchCount;
			}

			if(hasBitSet(RESPOND_LOWLEVEL, params))
			{
				if((int32_t)player->getLevel() > npcState->level)
					continue;
				++matchCount;
			}

			if(hasBitSet(RESPOND_LOWMONEY, params))
			{
				int32_t moneyCount = g_game.getMoney(player);
				if(moneyCount >= npcState->price)
					continue;
				++matchCount;
			}

			if(hasBitSet(RESPOND_LOWAMOUNT, params) || hasBitSet(RESPOND_NOAMOUNT, params))
			{
				int32_t itemCount = player->__getItemTypeCount(npcState->itemId);
				if(itemCount >= npcState->amount)
					continue;

				if(hasBitSet(RESPOND_LOWAMOUNT, params))
				{
					if(npcState->amount == 1)
						continue;
					++matchCount;
				}

				if(hasBitSet(RESPOND_NOAMOUNT, params))
				{
					if(npcState->amount > 1)
						continue;
					++matchCount;
				}
			}
		}

		if((*it)->getKnowSpell() != "")
		{
			std::string spellName = (*it)->getKnowSpell();
			if(spellName == "|SPELL|")
				spellName = npcState->spellName;

			if(!player->hasLearnedInstantSpell(spellName))
				continue;
			++matchCount;
		}

		if((*it)->scriptVars.b1)
		{
			if(!npcState->scriptVars.b1)
				continue;
			++matchCount;
		}

		if((*it)->scriptVars.b2)
		{
			if(!npcState->scriptVars.b2)
				continue;
			++matchCount;
		}

		if((*it)->scriptVars.b3)
		{
			if(!npcState->scriptVars.b3)
				continue;
			++matchCount;
		}

		if((*it)->getStorageId() != -1)
		{
			int32_t playerStorageValue = -1;
			if(!player->getStorageValue((*it)->getStorageId(), playerStorageValue))
				playerStorageValue = -1;

			int32_t storageValue = (*it)->getStorageValue();
			StorageComparision_t comp = (*it)->getStorageComp();
			switch(comp)
			{
				case STORAGE_LESS:
				{
					if(playerStorageValue >= storageValue)
						continue;
					break;
				}
				case STORAGE_LESSOREQUAL:
				{
					if(playerStorageValue > storageValue)
						continue;
					break;
				}
				case STORAGE_EQUAL:
				{
					if(playerStorageValue != storageValue)
						continue;
					break;
				}
				case STORAGE_GREATEROREQUAL:
				{
					if(playerStorageValue < storageValue)
						continue;
					break;
				}
				case STORAGE_GREATER:
				{
					if(playerStorageValue <= storageValue)
						continue;
					break;
				}
				default: break;
			}
			++matchCount;
		}

		if((*it)->getInteractType() == INTERACT_TEXT || (*it)->getFocusState() != -1)
		{
			if(npcState->isIdle && (*it)->getFocusState() != 1)
			{
				//We are idle, and this response does not activate the npc.
				continue;
			}

			if(!npcState->isIdle && (*it)->getFocusState() == 1)
			{
				//We are not idle, and this response would activate us again.
				continue;
			}
		}

		if(npcState->topic == -1 && (*it)->getTopic() != -1)
		{
			//Not the right topic
			continue;
		}

		if(npcState->topic != -1 && npcState->topic == (*it)->getTopic())
		{
			//Topic is right
			matchCount += 1000;
		}

		if((*it)->getInteractType() == INTERACT_EVENT)
		{
			if((*it)->getInputText() == asLowerCaseString(text))
				++matchCount;
			else
				matchCount = 0;
		}
		else if((*it)->getInteractType() == INTERACT_TEXT)
		{
			int32_t matchAllCount = 0; //Contains the number of keywords that where matched
			int32_t totalKeywordCount = 0; //Contains the total number of keywords that where being compared
			int32_t matchWordCount = getMatchCount(*it, wordList, exactMatch, matchAllCount, totalKeywordCount);

			if(matchWordCount> 0)
			{
				//Remove points for |*| matches
				matchWordCount -= matchAllCount;

				//Remove points for not full match
				matchWordCount -= (totalKeywordCount - matchAllCount - matchWordCount);

				//Total "points" for this response, word matches are worth more
				matchCount += matchWordCount * 100000;
			}
			else
				matchCount = 0;
		}

		if(matchCount > bestMatchCount)
		{
			totalMatchCount = 0;
			response = (*it);
			bestMatchCount = matchCount;
			//std::cout << "Found response string: " << response->getText() << ", keyword: " << response->getInputText() << std::endl;
		}
		else if(bestMatchCount > 0 && matchCount == bestMatchCount)
			++totalMatchCount;
	}

	if(totalMatchCount > 1)
		return NULL;

	return response;
}

uint32_t Npc::getMatchCount(NpcResponse* response, std::vector<std::string> wordList,
	bool exactMatch, int32_t& matchAllCount, int32_t& totalKeywordCount)
{
	matchAllCount = 0;
	totalKeywordCount = 0;
	int32_t bestMatchCount = 0;

	const std::list<std::string>& inputList = response->getInputList();
	for(std::list<std::string>::const_iterator it = inputList.begin(); it != inputList.end(); ++it)
	{
		int32_t matchCount = 0;
		std::vector<std::string>::iterator lastWordMatchIter = wordList.begin();
		std::string keywords = (*it);
		std::vector<std::string> keywordList = explodeString(keywords, ";");

		for(std::vector<std::string>::iterator keyIter = keywordList.begin(); keyIter != keywordList.end(); ++keyIter)
		{
			if(!exactMatch && (*keyIter) == "|*|")
			{
				//Match anything.
				matchAllCount++;
			}
			else if((*keyIter) == "|amount|")
			{
				//TODO: Should iterate through each word until a number or a new keyword is found.
				int32_t amount = atoi((*lastWordMatchIter).c_str());
				if(amount > 0)
					response->setAmount(amount);
				else
				{
					response->setAmount(1);
					continue;
				}
			}
			else
			{
				std::vector<std::string>::iterator wordIter = wordList.end();
				for(wordIter = lastWordMatchIter; wordIter != wordList.end(); ++wordIter)
				{
					size_t pos = (*wordIter).find_first_of("!\"#�%&/()=?`{[]}\\^*><,.-_'~");
					if(pos == std::string::npos)
						pos = 0;

					if((*wordIter).find((*keyIter), pos) == pos)
						break;
				}

				if(wordIter == wordList.end())
					continue;

				lastWordMatchIter = wordIter + 1;
			}

			++matchCount;

			if(matchCount > bestMatchCount)
			{
				bestMatchCount = matchCount;
				totalKeywordCount = keywordList.size();
			}

			if(lastWordMatchIter == wordList.end())
				break;
		}
	}
	return bestMatchCount;
}

const NpcResponse* Npc::getResponse(const Player* player, NpcState* npcState, const std::string& text)
{
	return getResponse(responseList, player, npcState, text);
}

const NpcResponse* Npc::getResponse(const Player* player, NpcEvent_t eventType)
{
	std::string eventName = getEventResponseName(eventType);
	if(eventName.empty())
		return NULL;

	std::vector<NpcResponse*> result;
	for(ResponseList::const_iterator it = responseList.begin(); it != responseList.end(); ++it)
	{
		if((*it)->getInteractType() == INTERACT_EVENT)
		{
			if((*it)->getInputText() == asLowerCaseString(eventName))
				result.push_back(*it);
		}
	}

	if(result.empty())
		return NULL;

	return result[random_range(0, result.size() - 1)];
}

const NpcResponse* Npc::getResponse(const Player* player, NpcState* npcState, NpcEvent_t eventType)
{
	std::string eventName = getEventResponseName(eventType);
	if(eventName.empty())
		return NULL;

	return getResponse(responseList, player, npcState, eventName, true);
}

std::string Npc::getEventResponseName(NpcEvent_t eventType)
{
	switch(eventType)
	{
		case EVENT_BUSY: return "onBusy"; break;
		case EVENT_THINK: return "onThink"; break;
		case EVENT_IDLE: return "onIdle"; break;
		case EVENT_PLAYER_ENTER: return "onPlayerEnter"; break;
		case EVENT_PLAYER_MOVE: return "onPlayerMove"; break;
		case EVENT_PLAYER_LEAVE: return "onPlayerLeave"; break;
		case EVENT_PLAYER_SHOPSELL: return "onPlayerShopSell"; break;
		case EVENT_PLAYER_SHOPBUY: return "onPlayerShopBuy"; break;
		case EVENT_PLAYER_SHOPCLOSE: return "onPlayerShopClose"; break;
		default: return ""; break;
	}
	return "";
}

std::string Npc::formatResponse(Creature* creature, const NpcState* npcState, const NpcResponse* response) const
{
	std::string responseString = response->getText();

	std::stringstream ss;
	ss << npcState->price * npcState->amount;
	replaceString(responseString, "|PRICE|", ss.str());

	ss.str("");
	ss << npcState->amount;
	replaceString(responseString, "|AMOUNT|", ss.str());

	ss.str("");
	ss << npcState->level;
	replaceString(responseString, "|LEVEL|", ss.str());

	ss.str("");
	ss << npcState->scriptVars.n1;
	replaceString(responseString, "|N1|", ss.str());

	ss.str("");
	ss << npcState->scriptVars.n2;
	replaceString(responseString, "|N2|", ss.str());

	ss.str("");
	ss << npcState->scriptVars.n3;
	replaceString(responseString, "|N3|", ss.str());

	replaceString(responseString, "|S1|", npcState->scriptVars.s1);
	replaceString(responseString, "|S2|", npcState->scriptVars.s2);
	replaceString(responseString, "|S3|", npcState->scriptVars.s3);

	ss.str("");
	if(npcState->itemId != -1)
	{
		const ItemType& it = Item::items[npcState->itemId];
		if(npcState->amount <= 1)
		{
			ss << it.article + " " + it.name;
			replaceString(responseString, "|ITEMNAME|", ss.str());
		}
		else
		{
			ss << it.pluralName;
			replaceString(responseString, "|ITEMNAME|", ss.str());
		}
	}

	replaceString(responseString, "|NAME|", creature->getName());
	replaceString(responseString, "|NPCNAME|", getName());
	return responseString;
}

void Npc::addShopPlayer(Player* player)
{
	ShopPlayerList::iterator it = std::find(shopPlayerList.begin(), shopPlayerList.end(), player);
	if(it == shopPlayerList.end())
		shopPlayerList.push_back(player);
}

void Npc::removeShopPlayer(const Player* player)
{
	ShopPlayerList::iterator it = std::find(shopPlayerList.begin(), shopPlayerList.end(), player);
	if(it != shopPlayerList.end())
		shopPlayerList.erase(it);
}

void Npc::closeAllShopWindows()
{
	ShopPlayerList closeList = shopPlayerList;
	for(ShopPlayerList::iterator it = closeList.begin(); it != closeList.end(); ++it)
		(*it)->closeShopWindow();
}

NpcScriptInterface* Npc::getScriptInterface()
{
	return m_scriptInterface;
}

NpcScriptInterface::NpcScriptInterface() :
	LuaScriptInterface("Npc interface")
{
	m_libLoaded = false;
	initState();
}


NpcScriptInterface::~NpcScriptInterface()
{
	//
}

bool NpcScriptInterface::initState()
{
	return LuaScriptInterface::initState();
}

bool NpcScriptInterface::closeState()
{
	m_libLoaded = false;
	return LuaScriptInterface::closeState();
}

bool NpcScriptInterface::loadNpcLib(std::string file)
{
	if(m_libLoaded)
		return true;

	if(loadFile(file) == -1)
	{
		std::cout << "Warning: [NpcScriptInterface::loadNpcLib] Can not load " << file << std::endl;
		return false;
	}

	m_libLoaded = true;
	return true;
}

void NpcScriptInterface::registerFunctions()
{
	LuaScriptInterface::registerFunctions();

	//npc exclusive functions
	lua_register(m_luaState, "selfSay", NpcScriptInterface::luaActionSay);
	lua_register(m_luaState, "selfMove", NpcScriptInterface::luaActionMove);
	lua_register(m_luaState, "selfMoveTo", NpcScriptInterface::luaActionMoveTo);
	lua_register(m_luaState, "selfTurn", NpcScriptInterface::luaActionTurn);
	lua_register(m_luaState, "selfFollow", NpcScriptInterface::luaActionFollow);
	lua_register(m_luaState, "selfGetPosition", NpcScriptInterface::luaSelfGetPos);
	lua_register(m_luaState, "creatureGetName", NpcScriptInterface::luaCreatureGetName);
	lua_register(m_luaState, "creatureGetName2", NpcScriptInterface::luaCreatureGetName2);
	lua_register(m_luaState, "creatureGetPosition", NpcScriptInterface::luaCreatureGetPos);
	lua_register(m_luaState, "getDistanceTo", NpcScriptInterface::luagetDistanceTo);
	lua_register(m_luaState, "doNpcSetCreatureFocus", NpcScriptInterface::luaSetNpcFocus);
	lua_register(m_luaState, "getNpcCid", NpcScriptInterface::luaGetNpcCid);
	lua_register(m_luaState, "getNpcPos", NpcScriptInterface::luaGetNpcPos);
	lua_register(m_luaState, "getNpcState", NpcScriptInterface::luaGetNpcState);
	lua_register(m_luaState, "setNpcState", NpcScriptInterface::luaSetNpcState);
	lua_register(m_luaState, "getNpcName", NpcScriptInterface::luaGetNpcName);
	lua_register(m_luaState, "getNpcParameter", NpcScriptInterface::luaGetNpcParameter);
	lua_register(m_luaState, "openShopWindow", NpcScriptInterface::luaOpenShopWindow);
	lua_register(m_luaState, "closeShopWindow", NpcScriptInterface::luaCloseShopWindow);
}

int32_t NpcScriptInterface::luaCreatureGetName2(lua_State* L)
{
	//creatureGetName2(name) - returns creature id
	popString(L);
	reportErrorFunc("Deprecated function.");
	lua_pushnil(L);
	return 1;
}

int32_t NpcScriptInterface::luaCreatureGetName(lua_State* L)
{
	//creatureGetName(cid)
	popNumber(L);
	reportErrorFunc("Deprecated function, use getCreatureName.");
	lua_pushstring(L, "");
	return 1;
}

int32_t NpcScriptInterface::luaCreatureGetPos(lua_State* L)
{
	//creatureGetPosition(cid)
	popNumber(L);
	reportErrorFunc("Deprecated function, use getCreaturePosition.");
	lua_pushnil(L);
	lua_pushnil(L);
	lua_pushnil(L);
	return 3;
}

int32_t NpcScriptInterface::luaSelfGetPos(lua_State* L)
{
	//selfGetPosition()
	ScriptEnviroment* env = getScriptEnv();
	Npc* npc = env->getNpc();
	if(npc)
	{
		Position pos = npc->getPosition();
		lua_pushnumber(L, pos.x);
		lua_pushnumber(L, pos.y);
		lua_pushnumber(L, pos.z);
	}
	else
	{
		lua_pushnil(L);
		lua_pushnil(L);
		lua_pushnil(L);
	}
	return 3;
}

int32_t NpcScriptInterface::luaActionSay(lua_State* L)
{
	//selfSay(words [[, target], send_to_all])
	// send_to_all defaults to true if there is no target, false otherwise
	uint32_t parameters = lua_gettop(L);
	uint32_t target = 0;
	bool send_to_all = true;

	if(parameters == 3)
	{
		send_to_all = (popNumber(L) == LUA_TRUE);
		target = popNumber(L);
	}
	else if(parameters == 2)
	{
		target = popNumber(L);
		send_to_all = false;
	}
	std::string msg(popString(L));

	ScriptEnviroment* env = getScriptEnv();

	Npc* npc = env->getNpc();
	Player* focus = env->getPlayerByUID(target);
	if(npc)
		npc->doSay(msg, focus, send_to_all);

	return 0;
}

int32_t NpcScriptInterface::luaActionMove(lua_State* L)
{
	//selfMove(direction)
	Direction dir = (Direction)popNumber(L);
	ScriptEnviroment* env = getScriptEnv();

	Npc* npc = env->getNpc();
	if(npc)
		npc->doMove(dir);

	return 0;
}

int32_t NpcScriptInterface::luaActionMoveTo(lua_State* L)
{
	//selfMoveTo(x,y,z)
	Position target;
	target.z = (uint8_t)popNumber(L);
	target.y = (uint16_t)popNumber(L);
	target.x = (uint16_t)popNumber(L);

	ScriptEnviroment* env = getScriptEnv();
	Npc* npc = env->getNpc();
	if(npc)
		npc->doMoveTo(target);

	return 0;
}

int32_t NpcScriptInterface::luaActionTurn(lua_State* L)
{
	//selfTurn(direction)
	Direction dir = (Direction)popNumber(L);

	ScriptEnviroment* env = getScriptEnv();

	Npc* npc = env->getNpc();
	if(npc)
		npc->doTurn(dir);

	return 0;
}

int32_t NpcScriptInterface::luaActionFollow(lua_State* L)
{
	//selfFollow(cid)
	uint32_t cid = popNumber(L);

	ScriptEnviroment* env = getScriptEnv();

	Player* player = env->getPlayerByUID(cid);
	if(cid != 0 && !player)
	{
		lua_pushboolean(L, false);
		return 1;
	}

	Npc* npc = env->getNpc();
	if(!npc)
	{
		lua_pushboolean(L, false);
		return 1;
	}

	bool result = npc->setFollowCreature(player, true);
	lua_pushboolean(L, result);
	return 1;
}

int32_t NpcScriptInterface::luagetDistanceTo(lua_State* L)
{
	//getDistanceTo(uid)
	uint32_t uid = popNumber(L);

	ScriptEnviroment* env = getScriptEnv();

	Npc* npc = env->getNpc();
	Thing* thing = env->getThingByUID(uid);
	if(thing && npc)
	{
		Position thing_pos = thing->getPosition();
		Position npc_pos = npc->getPosition();
		if(npc_pos.z != thing_pos.z)
			lua_pushnumber(L, -1);
		else
		{
			int32_t dist = std::max(std::abs(npc_pos.x - thing_pos.x), std::abs(npc_pos.y - thing_pos.y));
			lua_pushnumber(L, dist);
		}
	}
	else
	{
		reportErrorFunc(getErrorDesc(LUA_ERROR_THING_NOT_FOUND));
		lua_pushnil(L);
	}

	return 1;
}

int32_t NpcScriptInterface::luaSetNpcFocus(lua_State* L)
{
	//doNpcSetCreatureFocus(cid)
	uint32_t cid = popNumber(L);

	ScriptEnviroment* env = getScriptEnv();

	Npc* npc = env->getNpc();
	if(npc)
	{
		Creature* creature = env->getCreatureByUID(cid);
		if(creature)
			npc->hasScriptedFocus = true;
		else
			npc->hasScriptedFocus = false;

		npc->setCreatureFocus(creature);
	}
	return 0;
}

int32_t NpcScriptInterface::luaGetNpcPos(lua_State* L)
{
	//getNpcPos()
	ScriptEnviroment* env = getScriptEnv();

	Position pos(0, 0, 0);
	uint32_t stackpos = 0;

	Npc* npc = env->getNpc();
	if(npc)
	{
		pos = npc->getPosition();
		stackpos = npc->getParent()->__getIndexOfThing(npc);
	}

	pushPosition(L, pos, stackpos);
	return 1;
}

int32_t NpcScriptInterface::luaGetNpcState(lua_State* L)
{
	//getNpcState(cid)
	uint32_t cid = popNumber(L);

	ScriptEnviroment* env = getScriptEnv();
	const Player* player = env->getPlayerByUID(cid);
	if(!player)
	{
		lua_pushnil(L);
		return 1;
	}

	Npc* npc = env->getNpc();
	if(!npc)
	{
		lua_pushnil(L);
		return 1;
	}

	NpcState* state = npc->getState(player);
	NpcScriptInterface::pushState(L, state);
	return 1;
}

int32_t NpcScriptInterface::luaSetNpcState(lua_State* L)
{
	//setNpcState(state, cid)
	uint32_t cid = popNumber(L);

	ScriptEnviroment* env = getScriptEnv();
	const Player* player = env->getPlayerByUID(cid);
	if(!player)
	{
		lua_pushnil(L);
		return 1;
	}

	Npc* npc = env->getNpc();
	if(!npc)
	{
		lua_pushboolean(L, false);
		return 1;
	}

	NpcState* state = npc->getState(player);
	NpcScriptInterface::popState(L, state);
	lua_pushboolean(L, true);
	return 1;
}

int32_t NpcScriptInterface::luaGetNpcCid(lua_State* L)
{
	//getNpcCid()
	ScriptEnviroment* env = getScriptEnv();

	Npc* npc = env->getNpc();
	if(npc)
	{
		uint32_t cid = env->addThing(npc);
		lua_pushnumber(L, cid);
	}
	else
		lua_pushnil(L);

	return 1;
}

int32_t NpcScriptInterface::luaGetNpcName(lua_State* L)
{
	//getNpcName()
	ScriptEnviroment* env = getScriptEnv();

	Npc* npc = env->getNpc();
	if(npc)
		lua_pushstring(L, npc->getName().c_str());
	else
		lua_pushstring(L, "");

	return 1;
}

int32_t NpcScriptInterface::luaGetNpcParameter(lua_State* L)
{
	//getNpcParameter(paramKey)
	std::string paramKey = popString(L);

	ScriptEnviroment* env = getScriptEnv();

	Npc* npc = env->getNpc();
	if(npc)
	{
		Npc::ParametersMap::iterator it = npc->m_parameters.find(paramKey);
		if(it != npc->m_parameters.end())
			lua_pushstring(L, it->second.c_str());
		else
			lua_pushnil(L);
	}
	else
		lua_pushnil(L);

	return 1;
}

void NpcScriptInterface::pushState(lua_State* L, NpcState* state)
{
	lua_newtable(L);
	setField(L, "price", state->price);
	setField(L, "amount", state->amount);
	setField(L, "itemid", state->itemId);
	setField(L, "subtype", state->subType);
	setField(L, "topic", state->topic);
	setField(L, "level", state->level);
	setField(L, "spellname", state->spellName);
	setField(L, "listname", state->listName);
	setField(L, "listpname", state->listPluralName);
	setFieldBool(L, "isidle", state->isIdle);

	setField(L, "n1", state->scriptVars.n1);
	setField(L, "n2", state->scriptVars.n2);
	setField(L, "n3", state->scriptVars.n3);

	setFieldBool(L, "b1", state->scriptVars.b1);
	setFieldBool(L, "b2", state->scriptVars.b2);
	setFieldBool(L, "b3", state->scriptVars.b3);

	setField(L, "s1", state->scriptVars.s1);
	setField(L, "s2", state->scriptVars.s2);
	setField(L, "s3", state->scriptVars.s3);
}

void NpcScriptInterface::popState(lua_State* L, NpcState* &state)
{
	state->price = getField(L, "price");
	state->amount = getField(L, "amount");
	state->itemId = getField(L, "itemid");
	state->subType = getField(L, "subtype");
	state->topic = getField(L, "topic");
	state->level = getField(L, "level");
	state->spellName = getFieldString(L, "spellname");
	state->listName = getFieldString(L, "listname");
	state->listPluralName = getFieldString(L, "listpname");
	state->isIdle = getFieldBool(L, "isidle");

	state->scriptVars.n1 = getField(L, "n1");
	state->scriptVars.n2 = getField(L, "n2");
	state->scriptVars.n3 = getField(L, "n3");

	state->scriptVars.b1 = getFieldBool(L, "b1");
	state->scriptVars.b2 = getFieldBool(L, "b2");
	state->scriptVars.n3 = getFieldBool(L, "b3");

	state->scriptVars.s1 = getFieldString(L, "s1");
	state->scriptVars.s2 = getFieldString(L, "s2");
	state->scriptVars.s3 = getFieldString(L, "s3");
}

int32_t NpcScriptInterface::luaOpenShopWindow(lua_State* L)
{
	//openShopWindow(cid, items, onBuy callback, onSell callback)
	int32_t buyCallback = -1;
	int32_t sellCallback = -1;
	std::list<ShopInfo> items;
	Player* player = NULL;

	ScriptEnviroment* env = getScriptEnv();
	Npc* npc = env->getNpc();

	if(lua_isfunction(L, -1) == 0)
		lua_pop(L, 1); // skip it - use default value
	else
		sellCallback = popCallback(L);

	if(lua_isfunction(L, -1) == 0)
		lua_pop(L, 1);
	else
		buyCallback = popCallback(L);

	if(lua_istable(L, -1) == 0)
	{
		reportError(__FUNCTION__, "item list is not a table.");
		lua_pushnumber(L, LUA_ERROR);
		return 1;
	}

	ShopInfo item;
	// first key
	lua_pushnil(L);
	while(lua_next(L, -2) != 0)
	{
		item.itemId = getField(L, "id");
		item.subType = getField(L, "subType");
		item.buyPrice = getField(L, "buy");
		item.sellPrice = getField(L, "sell");
		item.itemName = getFieldString(L, "name");
		items.push_back(item);

		lua_pop(L, 1);
	}
	lua_pop(L, 1);

	player = env->getPlayerByUID(popNumber(L));
	if(!player)
	{
		reportErrorFunc(getErrorDesc(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushnumber(L, LUA_ERROR);
		return 1;
	}

	//Close any eventual other shop window currently open.
	player->closeShopWindow();

	if(!npc)
	{
		reportErrorFunc(getErrorDesc(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushnumber(L, LUA_ERROR);
		return 1;
	}

	npc->addShopPlayer(player);
	player->setShopOwner(npc, buyCallback, sellCallback);
	sortItems(items);
	player->sendShop(items);
	player->sendCash(g_game.getMoney(player));

	lua_pushnumber(L, LUA_NO_ERROR);
	return 1;
}

int32_t NpcScriptInterface::luaCloseShopWindow(lua_State* L)
{
	//closeShopWindow(cid)
	ScriptEnviroment* env = getScriptEnv();

	Player* player = env->getPlayerByUID(popNumber(L));
	if(!player)
	{
		reportErrorFunc(getErrorDesc(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushnumber(L, LUA_ERROR);
		return 1;
	}

	Npc* npc = env->getNpc();
	if(!npc)
	{
		reportErrorFunc(getErrorDesc(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushnumber(L, LUA_ERROR);
		return 1;
	}

	int32_t buyCallback;
	int32_t sellCallback;
	Npc* merchant = player->getShopOwner(buyCallback, sellCallback);

	//Check if we actually have a shop window with this player.
	if(merchant == npc)
	{
		player->sendCloseShop();

		if(buyCallback != -1)
			luaL_unref(L, LUA_REGISTRYINDEX, buyCallback);
		if(sellCallback != -1)
			luaL_unref(L, LUA_REGISTRYINDEX, sellCallback);

		player->setShopOwner(NULL, -1, -1);
		npc->removeShopPlayer(player);
	}
	return 1;
}

NpcEventsHandler::NpcEventsHandler(Npc* npc)
{
	m_npc = npc;
	m_loaded = false;
}

NpcEventsHandler::~NpcEventsHandler()
{
	//
}

bool NpcEventsHandler::isLoaded()
{
	return m_loaded;
}

NpcScript::NpcScript(std::string file, Npc* npc) :
	NpcEventsHandler(npc)
{
	m_scriptInterface = npc->getScriptInterface();

	if(m_scriptInterface->loadFile(file, npc) == -1)
	{
		std::cout << "[Warning - NpcScript::NpcScript] Can not load script: " << file << std::endl;
		std::cout << m_scriptInterface->getLastLuaError() << std::endl;
		m_loaded = false;
		return;
	}

	m_onCreatureSay = m_scriptInterface->getEvent("onCreatureSay");
	m_onCreatureDisappear = m_scriptInterface->getEvent("onCreatureDisappear");
	m_onCreatureAppear = m_scriptInterface->getEvent("onCreatureAppear");
	m_onCreatureMove = m_scriptInterface->getEvent("onCreatureMove");
	m_onPlayerCloseChannel = m_scriptInterface->getEvent("onPlayerCloseChannel");
	m_onPlayerEndTrade = m_scriptInterface->getEvent("onPlayerEndTrade");
	m_onThink = m_scriptInterface->getEvent("onThink");
	m_loaded = true;
}

NpcScript::~NpcScript()
{
	//
}

void NpcScript::onCreatureAppear(const Creature* creature)
{
	if(m_onCreatureAppear == -1)
		return;

	//onCreatureAppear(creature)
	if(m_scriptInterface->reserveScriptEnv())
	{
		ScriptEnviroment* env = m_scriptInterface->getScriptEnv();

		#ifdef __DEBUG_LUASCRIPTS__
		std::stringstream desc;
		desc << "npc " << m_npc->getName();
		env->setEventDesc(desc.str());
		#endif

		lua_State* L = m_scriptInterface->getLuaState();

		env->setScriptId(m_onCreatureAppear, m_scriptInterface);
		env->setRealPos(m_npc->getPosition());
		env->setNpc(m_npc);

		uint32_t cid = env->addThing(const_cast<Creature*>(creature));

		m_scriptInterface->pushFunction(m_onCreatureAppear);
		lua_pushnumber(L, cid);
		m_scriptInterface->callFunction(1);
		m_scriptInterface->releaseScriptEnv();
	}
	else
		std::cout << "[Error - NpcScript::onCreatureAppear] NPC Name: " << m_npc->getName() << " - Call stack overflow" << std::endl;
}

void NpcScript::onCreatureDisappear(const Creature* creature)
{
	if(m_onCreatureDisappear == -1)
		return;

	//onCreatureDisappear(id)
	if(m_scriptInterface->reserveScriptEnv())
	{
		ScriptEnviroment* env = m_scriptInterface->getScriptEnv();

		#ifdef __DEBUG_LUASCRIPTS__
		std::stringstream desc;
		desc << "npc " << m_npc->getName();
		env->setEventDesc(desc.str());
		#endif

		lua_State* L = m_scriptInterface->getLuaState();

		env->setScriptId(m_onCreatureDisappear, m_scriptInterface);
		env->setRealPos(m_npc->getPosition());
		env->setNpc(m_npc);

		uint32_t cid = env->addThing(const_cast<Creature*>(creature));

		m_scriptInterface->pushFunction(m_onCreatureDisappear);
		lua_pushnumber(L, cid);
		m_scriptInterface->callFunction(1);
		m_scriptInterface->releaseScriptEnv();
	}
	else
		std::cout << "[Error - NpcScript::onCreatureDisappear] NPC Name: " << m_npc->getName() << " - Call stack overflow" << std::endl;
}

void NpcScript::onCreatureMove(const Creature* creature, const Position& oldPos, const Position& newPos)
{
	if(m_onCreatureMove == -1)
		return;

	//onCreatureMove(creature, oldPos, newPos)
	if(m_scriptInterface->reserveScriptEnv())
	{
		ScriptEnviroment* env = m_scriptInterface->getScriptEnv();

		#ifdef __DEBUG_LUASCRIPTS__
		std::stringstream desc;
		desc << "npc " << m_npc->getName();
		env->setEventDesc(desc.str());
		#endif

		lua_State* L = m_scriptInterface->getLuaState();

		env->setScriptId(m_onCreatureMove, m_scriptInterface);
		env->setRealPos(m_npc->getPosition());
		env->setNpc(m_npc);

		uint32_t cid = env->addThing(const_cast<Creature*>(creature));

		m_scriptInterface->pushFunction(m_onCreatureMove);
		lua_pushnumber(L, cid);
		LuaScriptInterface::pushPosition(L, oldPos, 0);
		LuaScriptInterface::pushPosition(L, newPos, 0);
		m_scriptInterface->callFunction(3);
		m_scriptInterface->releaseScriptEnv();
	}
	else
		std::cout << "[Error - NpcScript::onCreatureMove] NPC Name: " << m_npc->getName() << " - Call stack overflow" << std::endl;
}

void NpcScript::onCreatureSay(const Creature* creature, SpeakClasses type, const std::string& text)
{
	if(m_onCreatureSay == -1)
		return;

	//onCreatureSay(cid, type, msg)
	if(m_scriptInterface->reserveScriptEnv())
	{
		ScriptEnviroment* env = m_scriptInterface->getScriptEnv();

		#ifdef __DEBUG_LUASCRIPTS__
		std::stringstream desc;
		desc << "npc " << m_npc->getName();
		env->setEventDesc(desc.str());
		#endif

		env->setScriptId(m_onCreatureSay, m_scriptInterface);
		env->setRealPos(m_npc->getPosition());
		env->setNpc(m_npc);

		uint32_t cid = env->addThing(const_cast<Creature*>(creature));

		lua_State* L = m_scriptInterface->getLuaState();
		m_scriptInterface->pushFunction(m_onCreatureSay);
		lua_pushnumber(L, cid);
		lua_pushnumber(L, type);
		lua_pushstring(L, text.c_str());
		m_scriptInterface->callFunction(3);
		m_scriptInterface->releaseScriptEnv();
	}
	else
		std::cout << "[Error - NpcScript::onCreatureSay] NPC Name: " << m_npc->getName() << " - Call stack overflow" << std::endl;
}

void NpcScript::onPlayerTrade(const Player* player, int32_t callback, uint16_t itemid,
	uint8_t count, uint8_t amount)
{
	if(callback == -1)
		return;

	//"onBuy"(cid, itemid, count, amount)
	if(m_scriptInterface->reserveScriptEnv())
	{
		ScriptEnviroment* env = m_scriptInterface->getScriptEnv();
		env->setScriptId(-1, m_scriptInterface);
		env->setRealPos(m_npc->getPosition());
		env->setNpc(m_npc);

		uint32_t cid = env->addThing(const_cast<Player*>(player));
		lua_State* L = m_scriptInterface->getLuaState();
		LuaScriptInterface::pushCallback(L, callback);
		lua_pushnumber(L, cid);
		lua_pushnumber(L, itemid);
		lua_pushnumber(L, count);
		lua_pushnumber(L, amount);
		m_scriptInterface->callFunction(4);
		m_scriptInterface->releaseScriptEnv();
	}
	else
		std::cout << "[Error - NpcScript::onPlayerTrade] NPC Name: " << m_npc->getName() << " - Call stack overflow" << std::endl;
}

void NpcScript::onPlayerCloseChannel(const Player* player)
{
	if(m_onPlayerCloseChannel == -1)
		return;

	//onPlayerCloseChannel(cid)
	if(m_scriptInterface->reserveScriptEnv())
	{
		ScriptEnviroment* env = m_scriptInterface->getScriptEnv();
		env->setScriptId(m_onPlayerCloseChannel, m_scriptInterface);
		env->setRealPos(m_npc->getPosition());
		env->setNpc(m_npc);

		uint32_t cid = env->addThing(const_cast<Player*>(player));

		lua_State* L = m_scriptInterface->getLuaState();
		m_scriptInterface->pushFunction(m_onPlayerCloseChannel);
		lua_pushnumber(L, cid);
		m_scriptInterface->callFunction(1);
		m_scriptInterface->releaseScriptEnv();
	}
	else
		std::cout << "[Error - NpcScript::onPlayerCloseChannel] NPC Name: " << m_npc->getName() << " - Call stack overflow" << std::endl;
}

void NpcScript::onPlayerEndTrade(const Player* player)
{
	if(m_onPlayerCloseChannel == -1)
		return;

	//onPlayerEndTrade(cid)
	if(m_scriptInterface->reserveScriptEnv())
	{
		ScriptEnviroment* env = m_scriptInterface->getScriptEnv();
		env->setScriptId(m_onPlayerCloseChannel, m_scriptInterface);
		env->setRealPos(m_npc->getPosition());
		env->setNpc(m_npc);

		uint32_t cid = env->addThing(const_cast<Player*>(player));

		lua_State* L = m_scriptInterface->getLuaState();
		m_scriptInterface->pushFunction(m_onPlayerEndTrade);
		lua_pushnumber(L, cid);
		m_scriptInterface->callFunction(1);
		m_scriptInterface->releaseScriptEnv();
	}
	else
		std::cout << "[Error - NpcScript::onPlayerEndTrade] NPC Name: " << m_npc->getName() << " - Call stack overflow" << std::endl;
}

void NpcScript::onThink()
{
	if(m_onThink == -1)
		return;

	//onThink()
	if(m_scriptInterface->reserveScriptEnv())
	{
		ScriptEnviroment* env = m_scriptInterface->getScriptEnv();

		#ifdef __DEBUG_LUASCRIPTS__
		std::stringstream desc;
		desc << "npc " << m_npc->getName();
		env->setEventDesc(desc.str());
		#endif

		env->setScriptId(m_onThink, m_scriptInterface);
		env->setRealPos(m_npc->getPosition());
		env->setNpc(m_npc);

		m_scriptInterface->pushFunction(m_onThink);
		m_scriptInterface->callFunction(0);
		m_scriptInterface->releaseScriptEnv();
	}
	else
		std::cout << "[Error - NpcScript::onThink] NPC Name: " << m_npc->getName() << " - Call stack overflow" << std::endl;
}
