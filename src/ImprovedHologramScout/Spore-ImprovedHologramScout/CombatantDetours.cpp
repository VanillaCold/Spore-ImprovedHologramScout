#include "stdafx.h"
#include "Detours.h"
#include <Spore/Simulator/SubSystem/TerraformingManager.h>
#include "HologramCombatManager.h";
#include "HologramScoutManager.h"

int OverrideCreatureDamageDetour::DETOUR(float damage, uint32_t attackerPoliticalID, int unk, const Vector3& unkPos, cCombatant* pAttacker)
{
	//Get the attacker, and verify it's the space-stage.
	if (Simulator::IsSpaceGame())
	{
		auto manager = HologramScoutManager::Get();
		auto combManager = HologramCombatManager::Get();
		if (this == (Simulator::cCombatant*)GameNounManager.GetAvatar() && manager->mbWasActive)
		{
			mMaxHealthPoints = manager->mMaxPlayerHealth;
		}

		if (pAttacker)
		{
			//Cast the attacker to a creature.
			auto creature = object_cast<Simulator::cCreatureBase>(pAttacker);
			if (creature) //if it is one,
			{
				//Get the current ability index being used,
				uint32_t attackIndex = creature->mCurrentAttackIdx;
				//and get the ability from that.
				cCreatureAbilityPtr ability = creature->GetAbility(attackIndex);
				//If it's a valid ability,
				if (ability)
				{
					//set the damage to ability->mDamage, so that it isn't overpowered.
					damage = ability->mDamage;
				}
				else
				{
					ability = HologramCombatManager::Get()->GetLastAbilityUsed(creature);
					if (ability)
					{
						damage = ability->mDamage;
					}
				}
			}
		}
		else if (GameNounManager.GetAvatar() && int(damage) == 9999)
		{
			//just pickup the last creature - assume it's caused by that.
			cCreatureBasePtr crt = HologramCombatManager::Get()->mpLastCreatureToAttack;

			if (attackerPoliticalID == GameNounManager.GetAvatar()->mPoliticalID)
			{
				SporeDebugPrint("0x%x", GameNounManager.GetAvatar()->mPoliticalID);
				crt = GameNounManager.GetAvatar();
			}

			else if (crt.get() == this)
			{
				//This code runs if the last creature to attack is the one being targetted.
				//If there is one, assume that this combatant's target is the attacker.
				crt = object_cast<Simulator::cCreatureBase>(this->mpCombatantTarget);
			}


			if (crt)
			{
				//get the new ability
				auto ability = HologramCombatManager::Get()->GetLastAbilityUsed(crt);
				if (ability)
				{
					damage = ability->mDamage;
				}
			}
			else
			{
				//Just... assume the damage is something like 10.
				damage = 10;
			}
		}
		SporeDebugPrint("Damage is %f, caused by political ID of 0x%x", damage, attackerPoliticalID);

	}
	return original_function(this, damage, attackerPoliticalID, unk, unkPos, pAttacker);
}

void PlayAbilityDetour::DETOUR(int abilityIndex, Anim::AnimIndex* dstAnimIndex)
{
	if (Simulator::IsSpaceGame() && !Simulator::IsScenarioMode())
	{
		AbilityUsedData* data = new AbilityUsedData();
		auto ability = this->GetAbility(abilityIndex);
		if (ability)
		{
			if (ability->mType == 29)
			{
				delete data;
				return original_function(this, abilityIndex, dstAnimIndex);
			}

			data->mAbilityKey = ability->mpPropList->GetResourceKey();
			//data->mpAbility = ability;
			data->mpCreatureTarget = this->mpCombatantTarget;
			data->mpSourceCreature = this;
			MessageManager.MessageSend(id("SpaceGameAttackused"), data);
		}
	}
	return original_function(this, abilityIndex, dstAnimIndex);
}