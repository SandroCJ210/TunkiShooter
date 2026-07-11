# Checklist de editor — MVP AmaruShooter

Orden recomendado. Prerrequisito: el proyecto compila.

## 1. GameplayTag nuevo (solo si Atoq tendrá arma propia)
- [ ] Project Settings → Project → GameplayTags → Add: `Weapon.Range.Crossbow`.
      (Si Atoq va a reusar el bastón, salta este paso y usa `Weapon.Range.Staff`.)

## 2. GE_Init por héroe (Content/GAS/GameplayEffects)
Los tres deben inicializar (modifiers Override, como ya hacen):
`Health/MaxHealth`, `Shield=0 / MaxShield=300`, `MoveSpeed`, `DamageMultiplier=1`,
`Ammo/MaxAmmo`, `ChargeAbility1-2/Max`, y **nuevo**: `UltimateCharge=0 / MaxUltimateCharge=1000`.
- [ ] `GE_Init_Pachamama` (tank): MaxHealth ~250, MoveSpeed ~450, MaxAmmo ~8. Añadir MaxShield/MaxUltimateCharge.
- [ ] `GE_Init_Wayra` (support): MaxHealth ~180, MoveSpeed ~600, MaxAmmo ~10. Añadir MaxShield/MaxUltimateCharge.
- [ ] `GE_Init_Atoq`: duplicar GE_Init_Wayra → MaxHealth ~150, MoveSpeed ~650, MaxAmmo ~12.

## 3. Actores estéticos (BP hijos de clases C++, solo mesh/VFX)
- [ ] `BP_PachamamaOrb` (parent `PachamamaOrb`): asignar Static Mesh al componente Mesh + material tierra (VFX opcional: NS_GreenFire como hijo).
- [ ] `BP_WayraOrb` (parent `WayraOrb`): mesh + NS_Projectile_GreenFire.
- [ ] `BP_PachaWall` (parent `PachamamaWall`): mesh de muro (escalar el cubo básico sirve).
- [ ] `BP_PachaDome` (parent `PachamamaDome`): mesh semiesfera con material MI_ShieldBubble; ajustar radio del componente Zone al mesh.
- [ ] `BP_HealthPack` (parent `HealthPack`): mesh (cruz/botiquín).

## 4. Habilidades que spawnean actores (BP hijos que solo re-apuntan la clase)
Las GA C++ spawnean la clase C++ sin mesh; el BP hijo cambia una propiedad:
- [ ] `BP_GA_PachaPrimary` (parent `AmaruGA_PachaPrimary`): ProjectileClass = BP_PachamamaOrb.
- [ ] `BP_GA_PachaWall` (parent `AmaruGA_PachaWall`): WallClass = BP_PachaWall.
- [ ] `BP_GA_PachaDome` (parent `AmaruGA_PachaDome`): DomeClass = BP_PachaDome.
- [ ] `BP_GA_WayraPrimary` (parent `AmaruGA_WayraPrimary`): ProjectileClass = BP_WayraOrb.
(Atoq es hitscan: no necesita BP.)

## 5. DataAssets de héroes (Content/GAS/DataAssets)
CooldownEffect de cada entrada = **None** (las C++ se autogestionan). InputIDs: Ability1=1, Ability2=2, Ultimate=3, PrimaryFire=4, Reload=5.
- [ ] `DA_Atoq` (nuevo, tipo InkaDataAsset):
      Abilities = AmaruGA_AtoqPrimary(4), AmaruGA_AtoqFocus(1), AmaruGA_AtoqStealth(2), AmaruGA_AtoqRage(3), GA_Reload(5).
      StartupEffects = GE_Init_Atoq. WeaponTag = Weapon.Range.Staff (o Crossbow).
- [ ] `Pachamama_DataAsset` (re-apuntar):
      BP_GA_PachaPrimary(4), AmaruGA_PachaBubble(1), BP_GA_PachaWall(2), BP_GA_PachaDome(3), GA_Reload(5). StartupEffects = GE_Init_Pachamama.
- [ ] `Wayra_DataAsset` (re-apuntar):
      BP_GA_WayraPrimary(4), AmaruGA_WayraHeal(1), AmaruGA_WayraDash(2), AmaruGA_WayraUlt(3), GA_Reload(5). StartupEffects = GE_Init_Wayra.

## 6. Arma de Atoq
- [ ] `BP_CrossbowWeapon`: duplicar BP_StaffWeapon; WeaponConfig → AbilitiesToGrant **vacío** (el disparo viene del DataAsset), mismo socket, AnimationClass del staff (placeholder).
- [ ] En el flujo de GA_Equip_Weapon (donde se mapea WeaponTag → clase de arma): añadir Weapon.Range.Crossbow → BP_CrossbowWeapon. Si reusaste Weapon.Range.Staff, nada que hacer.

## 7. Character BP (BP_FirstPersonCharacter)
- [ ] Capsule/Mesh: verificar que bloquea canal `Visibility` (hitscan) y `Projectile` (impactos).
- [ ] Verificar que tiene el `WeaponManagerComponent` y `EquipWeaponAbility` asignada.
- [ ] Implementar evento `OnStealthChanged(bool)`: material translúcido en Mesh1P/arma al entrar en stealth (solo lo ve el dueño; a los rivales el C++ ya les oculta el Mesh3P).
- [ ] Verificar que GA_Reload y GA_Equip_Weapon NO sobreescriben ActivationBlockedTags (para heredar el bloqueo por State.Dead).

## 8. HUD y widgets
- [ ] `WBP_InkaHUD` → File → Reparent Blueprint → `AmaruHUDWidget`. Borrar la función `SetupAbilitySystemComponent` y su wiring.
- [ ] Implementar eventos: OnHUDReady (cargar iconos del loadout), OnHealthChanged, OnShieldChanged, OnAmmoChanged, OnUltimateChargeChanged (ult meter), OnAbilityChargeChanged, OnTeamAssigned (color de marco).
- [ ] Timer de partida: bindear texto a `GetMatchTimeRemaining` (formatear mm:ss).
- [ ] Panel Convoy: bindear a `GetConvoyPhase`, `GetCaptureProgress`, `GetConvoyProgress`, `IsConvoyMoving`, `IsLocalPlayerAttacking`, `GetCurrentRound`.
- [ ] Cooldowns en `WBP_InkaAbility`: variable FGameplayTag por instancia + bindear el radial a `GetCooldownRemaining` del HUD padre (tags: Cooldown.Atoq.Ability1, Cooldown.Pachamama.Ability2, etc.).
- [ ] `WBP_Scoreboard` (nuevo, parent `AmaruScoreboardWidget`): filas desde `GetScoreRows()` (refrescar al mostrarse o timer 0.5s). Mostrarlo con Tab.
- [ ] Panel de victoria: visibilidad bindeada a `GetWinningTeam() != -1` o `GetWinningPlayer()` válido; mostrar nombre/equipo + scoreboard.
- [ ] `DT_InkaTextureData`: añadir fila de Atoq (marco/retrato) si el HUD la usa.

## 9. Selección de héroe (lobby)
- [ ] `W_InkaSelect` / `W_InkaSelection`: añadir tercera opción → `SetSelectedInka(DA_Atoq)`.

## 10. Mapas
Convoy (duplicar FirstPersonMap → `L_Convoy`):
- [ ] World Settings → GameMode Override = `AmaruGM_Convoy`.
- [ ] Colocar 1 `ACapturePoint` (ajustar el Box de la zona).
- [ ] Colocar 1 `AConvoyCart`: editar el spline Track por el recorrido (los puntos intermedios se vuelven checkpoints automáticamente; o setear CheckpointDistances a mano), asignar mesh al CartMesh en la instancia.
- [ ] 3–5 `BP_HealthPack` por el mapa.
- [ ] PlayerStarts: 3+ con PlayerStartTag = `Attack` (cerca del punto), 3+ con `Defense`.
Deathmatch (`L_Deathmatch` o el propio FirstPersonMap):
- [ ] GameMode Override = `AmaruGM_Deathmatch`, PlayerStarts repartidos (sin tag), healthpacks.
General:
- [ ] Añadir los mapas nuevos a Project Settings → Packaging → List of maps to cook.
- [ ] Lobby: revisar el parent de `GM_Lobby` — si hereda de AmaruGameMode, reparentarlo a GameModeBase (si no, el timer de partida corre en el lobby). Verificar que el travel del lobby apunte al mapa nuevo.

## 11. Prueba PIE (2+ jugadores, Net Mode = Listen Server)
- [ ] DM: matarse → K/D suben, respawn a los 5s con vida llena, sin crash tras varios respawns, solo arma (sin habilidades), ganador al expirar el tiempo.
- [ ] Atoq: hitscan daña y sube Accuracy; stealth oculta al rival y acelera; ult solo con carga llena; cura al matar en rage.
- [ ] Pachamama: orbe explota en área; burbuja da escudo a aliado apuntado; muro da escudo al dañar cerca; domo reduce daño aliado y ralentiza enemigos.
- [ ] Wayra: orbe reduce CD de curación al acertar; cura al aliado con menos vida; dash con protección; ult cura + haste a todos.
- [ ] Convoy: captura avanza solo con atacantes (contested pausa), carro se mueve con escolta y sin defensores, checkpoint suma tiempo, ronda 2 invierte roles y spawns, ganador correcto.
- [ ] Friendly fire bloqueado en Convoy; healthpack cura y reaparece a los 15s.
