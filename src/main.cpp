

namespace Hooks
{
	namespace
	{
		// All other form types either handle their own editor ID or run into bugs
		// if we add their editor ID to the main table.
		const static std::vector<REL::Offset> kSupportedForms{
			RE::VTABLE::TESClass.front(),
			RE::VTABLE::TESFaction.front(),
			RE::VTABLE::TESHair.front(),
			RE::VTABLE::TESEyes.front(),
			RE::VTABLE::TESRace.front(),
			RE::VTABLE::TESSkill.front(),
			RE::VTABLE::EffectSetting.front(),
			RE::VTABLE::Script.front(),
			RE::VTABLE::TESLandTexture.front(),
			RE::VTABLE::EnchantmentItem.front(),
			RE::VTABLE::SpellItem.front(),
			RE::VTABLE::BirthSign.front(),
			RE::VTABLE::TESObjectACTI.front(),
			RE::VTABLE::TESObjectAPPA.front(),
			RE::VTABLE::TESObjectARMO.front(),
			RE::VTABLE::TESObjectBOOK.front(),
			RE::VTABLE::TESObjectCLOT.front(),
			RE::VTABLE::TESObjectCONT.front(),
			RE::VTABLE::TESObjectDOOR.front(),
			RE::VTABLE::IngredientItem.front(),
			RE::VTABLE::TESObjectLIGH.front(),
			RE::VTABLE::TESObjectMISC.front(),
			RE::VTABLE::TESObjectSTAT.front(),
			RE::VTABLE::TESGrass.front(),
			RE::VTABLE::TESObjectTREE.front(),
			RE::VTABLE::TESFlora.at(1),
			RE::VTABLE::TESFurniture.front(),
			RE::VTABLE::TESObjectWEAP.front(),
			RE::VTABLE::TESAmmo.front(),
			RE::VTABLE::TESNPC.front(),
			RE::VTABLE::TESCreature.front(),
			RE::VTABLE::TESLevCreature.front(),
			RE::VTABLE::TESSoulGem.front(),
			RE::VTABLE::TESKey.front(),
			RE::VTABLE::AlchemyItem.front(),
			RE::VTABLE::TESSubSpace.front(),
			RE::VTABLE::TESSigilStone.front(),
			RE::VTABLE::TESLevItem.front(),
			RE::VTABLE::TESWeather.front(),
			RE::VTABLE::TESClimate.front(),
			RE::VTABLE::TESRegion.front(),
			RE::VTABLE::TESObjectREFR.front(),
			RE::VTABLE::TESIdleForm.front(),
			RE::VTABLE::TESPackage.front(),
			RE::VTABLE::TESCombatStyle.front(),
			RE::VTABLE::TESLoadScreen.front(),
			RE::VTABLE::TESLevSpell.front(),
			RE::VTABLE::TESObjectANIO.front(),
			RE::VTABLE::TESWaterForm.front(),
			RE::VTABLE::TESEffectShader.front(),
			RE::VTABLE::Character.front(),
			RE::VTABLE::Actor.front(),
			RE::VTABLE::Creature.front(),
		};

		// The game's editor ID map doesn't copy keys, so we can get away with 
		// passing direct pointers to our backing store. And the internal table 
		// is only mutated during plugin load and doesn't use any locking.
		static std::unordered_set<std::string> EditorIdBackingStore;

		std::string ToLower(const std::string_view& input)
		{
			std::string result(input);
			std::transform(input.begin(), input.end(), result.begin(), [](unsigned char c)
			               { return std::tolower(c); });
			return result;
		}

		// Actually BSTCaseInsensitiveStringMap<TESForm *>
		struct TESFormEditorIdTable
		{
			void** vtbl;
			// ...

			void* Insert(const char* editorId, void* form)
			{
				using func_t = decltype(&TESFormEditorIdTable::Insert);
				static REL::Relocation<func_t> func{ REL::Offset(0x6656EC0) };
				return func(this, editorId, form);
			}

			static TESFormEditorIdTable* GetSingleton()
			{
				static REL::Relocation<TESFormEditorIdTable*> obj{ REL::Offset(0x8FDDD70) };
				return obj.get();
			}
		};


		static bool Hook_TESForm_SetEditorID(void* form, const char* const editorId) 
		{
			const auto key_lower{ ToLower(editorId) };
			const auto match{
				EditorIdBackingStore.find(key_lower)
			};
			if (match != EditorIdBackingStore.end())
			{
				REX::WARN("Duplicate editor ID: {} - Overriding form...", editorId);
				TESFormEditorIdTable::GetSingleton()->Insert(match->c_str(), form);
			}
			else
			{
				const auto result{ EditorIdBackingStore.insert(key_lower) };
				TESFormEditorIdTable::GetSingleton()->Insert(result.first->c_str(), form);
				//REX::INFO("Inserted editorID {}", editorId);
			}
			return true;
		}
	}

	static void Install()
	{
		for (const auto& vtbl : kSupportedForms)
		{
			REL::Relocation<std::uintptr_t> patch{ vtbl };
			//REX::INFO("Installed hook for vtbl 0x{:08X}", patch.address());
			patch.write_vfunc(0x36, Hooks::Hook_TESForm_SetEditorID);
		}

		REX::INFO("Installed hooks");
	}
}
 

namespace
{
	void MessageHandler(OBSE::MessagingInterface::Message* a_msg)
	{
		switch (a_msg->type)
		{
		case OBSE::MessagingInterface::kPostLoad:
			REX::INFO("PostLoad");
			break;
		case OBSE::MessagingInterface::kPostPostLoad:
			REX::INFO("PostPostLoad");
			break;
		default:
			break;
		}
	}
}

OBSE_PLUGIN_PRELOAD(const OBSE::PreLoadInterface* a_obse)
{
	OBSE::Init(a_obse);
	REX::INFO("Preload");
	return true;
}

OBSE_PLUGIN_LOAD(const OBSE::LoadInterface* a_obse)
{
	OBSE::Init(a_obse);
	OBSE::GetMessagingInterface()->RegisterListener(MessageHandler);
	Hooks::Install();
	REX::INFO("Load");
	return true;
}
