#pragma once

#include "../Memory.h"
#include "../Interfaces.h"
#include "../SDK/FrameStage.h"
#include "../SDK/UserCmd.h"
#include "../SDK/WeaponId.h"
#include "../SDK/NetworkChannel.h"
#include "../SDK/Entity.h"
#include "../SDK/Client.h"
#include "../SDK/GameEvent.h"
#include "../SDK/GameMovement.h"
#include "../SDK/GlobalVars.h"
#include "../SDK/Surface.h"
#include "../SDK/ConVar.h"
#include "../SDK/ViewSetup.h"
#include "../SDK/Input.h"

namespace Misc {
    void AutoBlocker(UserCmd*) noexcept;
    void slowwalk(UserCmd* cmd) noexcept;
    void inverseRagdollGravity() noexcept;
    void updateClanTag(bool = false) noexcept;
    void spectatorList() noexcept;
    void sniperCrosshair() noexcept;
    void recoilCrosshair() noexcept;
    void watermark() noexcept;
    void prepareRevolver(UserCmd*) noexcept;
    void fastPlant(UserCmd*) noexcept;
    void drawBombTimer() noexcept;
    void stealNames() noexcept;
    void quickReload(UserCmd*) noexcept;
    bool changeName(bool, const char*, float) noexcept;
    void chatSpam() noexcept;
    void fakeVote(bool = false) noexcept;
	void bunnyHop(UserCmd*) noexcept;
    void fakeBan(bool = false) noexcept;
    void nadePredict() noexcept;
    void quickHealthshot(UserCmd*) noexcept;
    void fixTabletSignal() noexcept;
    void fakePrime() noexcept;

    constexpr void fixMovement(UserCmd* cmd, float yaw) noexcept
    {
        if (config.misc.fixMovement) {
            float oldYaw = yaw + (yaw < 0.0f ? 360.0f : 0.0f);
            float newYaw = cmd->viewangles.y + (cmd->viewangles.y < 0.0f ? 360.0f : 0.0f);
            float yawDelta = newYaw < oldYaw ? fabsf(newYaw - oldYaw) : 360.0f - fabsf(newYaw - oldYaw);
            yawDelta = 360.0f - yawDelta;

            const float forwardmove = cmd->forwardmove;
            const float sidemove = cmd->sidemove;
            cmd->forwardmove = std::clamp(cos(degreesToRadians(yawDelta)) * forwardmove + cos(degreesToRadians(yawDelta + 90.0f)) * sidemove, -450.0f, 450.0f);
            cmd->sidemove = std::clamp(sin(degreesToRadians(yawDelta)) * forwardmove + sin(degreesToRadians(yawDelta + 90.0f)) * sidemove, -450.0f, 450.0f);
        }
    }

    constexpr void antiAfkKick(UserCmd* cmd) noexcept
    {
        if (config.misc.antiAfkKick && cmd->commandNumber % 2)
            cmd->buttons |= 1 << 26;
    }

    constexpr void fixAnimationLOD(FrameStage stage) noexcept
    {
        if (config.misc.fixAnimationLOD && stage == FrameStage::RENDER_START) {
			for (int i = 1; i <= interfaces.engine->getMaxClients(); ++i) {
                if (i == interfaces.engine->getLocalPlayer()) continue;
                Entity* entity = interfaces.entityList->getEntity(i);
                if (!entity || entity->isDormant() || !entity->isAlive()) continue;
                *reinterpret_cast<int*>(entity + 0xA28) = 0;
                *reinterpret_cast<int*>(entity + 0xA30) = memory.globalVars->framecount;
            }
        }
    }

    constexpr void autoPistol(UserCmd* cmd) noexcept
    {
        if (config.misc.autoPistol) {
            const auto activeWeapon = interfaces.entityList->getEntity(interfaces.engine->getLocalPlayer())->getActiveWeapon();
            if (activeWeapon && activeWeapon->isPistol() && activeWeapon->nextPrimaryAttack() > memory.globalVars->serverTime()) {
                if (activeWeapon->itemDefinitionIndex2() == WeaponId::Revolver)
                    cmd->buttons &= ~UserCmd::IN_ATTACK2;
                else
                    cmd->buttons &= ~UserCmd::IN_ATTACK;
            }
        }
    }

    constexpr void chokePackets(bool& sendPacket) noexcept
    {
        if (!config.misc.chokedPacketsKey || GetAsyncKeyState(config.misc.chokedPacketsKey))
            sendPacket = interfaces.engine->getNetworkChannel()->chokedPackets >= config.misc.chokedPackets;
    }

    constexpr void autoReload(UserCmd* cmd) noexcept
    {
        if (config.misc.autoReload) {
            const auto activeWeapon = interfaces.entityList->getEntity(interfaces.engine->getLocalPlayer())->getActiveWeapon();
            if (activeWeapon && getWeaponIndex(activeWeapon->itemDefinitionIndex2()) && !activeWeapon->clip())
                cmd->buttons &= ~(UserCmd::IN_ATTACK | UserCmd::IN_ATTACK2);
        }
    }

    constexpr void revealRanks(UserCmd* cmd) noexcept
    {
        if (config.misc.revealRanks && cmd->buttons & UserCmd::IN_SCORE)
            interfaces.client->dispatchUserMessage(50, 0, 0, nullptr);
    }

    static void usespam(UserCmd* cmd) noexcept
    {
        auto localPlayer = interfaces.entityList->getEntity(interfaces.engine->getLocalPlayer());
        static bool usespam = true;
        if (config.misc.usespam && cmd->buttons & UserCmd::IN_USE) {
            if (usespam)
            {
                cmd->buttons |= UserCmd::IN_USE;
                usespam = false;
            }
            else
            {
                cmd->buttons &= ~UserCmd::IN_USE;
                usespam = true;
            }
        }
    }
    static void autoStrafe(UserCmd* cmd) noexcept
    {
        auto localPlayer = interfaces.entityList->getEntity(interfaces.engine->getLocalPlayer());
        static bool bDirection = true;
        static auto wasLastTimeOnGround{ localPlayer->flags() & 1 };
        float flYawBhop = 0.f;
        if (localPlayer->velocity().length() > 50.f)
        {
            float x = 30.f;
            float y = localPlayer->velocity().length();
            float z = 0.f;
            float a = 0.f;

            z = x / y;
            z = fabsf(z);

            a = x * z;

            flYawBhop = a;
        }
        if (config.misc.autoStrafe && localPlayer->moveType() != MoveType::LADDER && !wasLastTimeOnGround && !(localPlayer->flags() & 1) && config.misc.autostrafestyle == 0) {
            if (cmd->mousedx > 1)
            {
                cmd->sidemove = 450.f;
            }
            else
            {
                cmd->sidemove = -450.f;
            }
        }
        if (config.misc.autoStrafe && localPlayer->moveType() != MoveType::LADDER && !wasLastTimeOnGround && !(localPlayer->flags() & 1) && config.misc.autostrafestyle == 1) {
            if (bDirection || cmd->mousedx > 1)
            {
                cmd->viewangles.y -= flYawBhop;
                cmd->sidemove = -450.f;
                bDirection = false;
            }
            else
            {
                cmd->viewangles.y += flYawBhop;
                cmd->sidemove = 450.f;
                bDirection = true;
            }
        }
        wasLastTimeOnGround = localPlayer->flags() & 1;
        //CorrectMovement(cmd->viewangles, cmd, cmd->forwardmove, cmd->sidemove);
    }

    constexpr void removeCrouchCooldown(UserCmd* cmd) noexcept
    {
        if (config.misc.fastDuck)
            cmd->buttons |= UserCmd::IN_BULLRUSH;
    }

    constexpr void moonwalk(UserCmd* cmd) noexcept
    {
        if (config.misc.moonwalk && interfaces.entityList->getEntity(interfaces.engine->getLocalPlayer())->moveType() != MoveType::LADDER)
            cmd->buttons ^= UserCmd::IN_FORWARD | UserCmd::IN_BACK | UserCmd::IN_MOVELEFT | UserCmd::IN_MOVERIGHT;
    }

    constexpr void playHitSound(GameEvent* event) noexcept
    {
        constexpr const char* hitSounds[]{
            "play physics/metal/metal_solid_impact_bullet2",
            "play buttons/arena_switch_press_02",
            "play training/timer_bell",
            "play physics/glass/glass_impact_bullet1",
            "play bubble",
            "play cod",
            "play fatality",
            "play custom",

        };

        if (config.misc.hitSound
            && interfaces.engine->getPlayerForUserID(event->getInt("attacker")) == interfaces.engine->getLocalPlayer())
            interfaces.engine->clientCmdUnrestricted(hitSounds[config.misc.hitSound - 1]);
    }

    constexpr void killMessage(GameEvent* event) noexcept
    {
        auto localPlayer = interfaces.engine->getLocalPlayer();
        if (config.misc.killMessage
            && interfaces.engine->getPlayerForUserID(event->getInt("attacker")) == localPlayer
            && interfaces.engine->getPlayerForUserID(event->getInt("userid")) != localPlayer)
            interfaces.engine->clientCmdUnrestricted(std::string{ "say " }.append(config.misc.killMessageString).c_str());
	}

	constexpr void fakeDuck(UserCmd* cmd) noexcept
	{

		if (config.misc.fakeDuckKey
			&& GetAsyncKeyState(config.misc.fakeDuckKey))
			if (const auto localPlayer{ interfaces.entityList->getEntity(interfaces.engine->getLocalPlayer()) };
				localPlayer
				&& localPlayer->isAlive())
				if (interfaces.engine->getNetworkChannel()->chokedPackets > (config.misc.chokedPackets / 2))
					cmd->buttons |= UserCmd::IN_DUCK;
				else
					cmd->buttons &= ~UserCmd::IN_DUCK;
	}

	constexpr void fakeDuckFix(ViewSetup* setup) noexcept
	{
		if (config.misc.fakeDuckKey
			&& GetAsyncKeyState(config.misc.fakeDuckKey))
			if (const auto localPlayer{ interfaces.entityList->getEntity(interfaces.engine->getLocalPlayer()) };
				localPlayer
				&& localPlayer->isAlive()
				&& localPlayer->flags() & 1)
				setup->origin.z = localPlayer->getAbsOrigin().z + interfaces.gameMovement->getPlayerViewOffset(false).z;
	}
}

