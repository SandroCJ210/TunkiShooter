# AmaruShooter - Arquitectura y Roadmap

## Arquitectura actual

### Stack

- Unreal Engine 5.6
- Gameplay Ability System
- Enhanced Input
- OnlineSubsystem + Steam
- UI mayormente en Blueprint

### Flujo actual

1. `AAmaruPlayerState` aloja `UAmaruAbilitySystemComponent` y `UAmaruAttributeSet`.
2. `SelectedInka` replica el personaje/loadout elegido.
3. `AAmaruShooterCharacter` inicializa GAS al poseer o replicar `PlayerState`.
4. `UInkaDataAsset` define startup effects, habilidades y `WeaponTag`.
5. `AAmaruShooterCharacter` aplica effects, otorga abilities y retransmite cambios a UI.
6. `UWeaponManagerComponent` equipa un arma fisica y entrega abilities de arma.
7. `AAmaruPlayerController` crea HUD y lo conecta al ASC usando una funcion Blueprint por nombre.

### Problemas estructurales actuales

- `AAmaruShooterCharacter` concentra demasiadas responsabilidades.
- La logica de equipamiento y grant/clear de abilities de arma estaba incompleta.
- La integracion UI depende de `ProcessEvent`, que es fragil y dificil de mantener.
- El proyecto depende de `LoadSynchronous` para contenido clave en runtime.
- No hay una capa formal de "ability sets" ni "loadout sets".

## Arquitectura futura recomendada

### Objetivo

Separar gameplay, loadout, armas y UI en modulos claros con ownership de red explicito.

### Componentes propuestos

- `AAmaruPlayerState`
  - Mantener `ASC`, `AttributeSet` y estado replicado del jugador.
- `UAmaruLoadoutComponent`
  - Resolver `SelectedInka`, precarga de assets, startup effects y grant base de abilities.
- `UWeaponManagerComponent`
  - Limitarse a equipar/desequipar armas, sockets, mesh, anim class y weapon abilities.
- `UAmaruWidgetController` o interfaz UI
  - Traducir cambios de GAS a datos de HUD sin `ProcessEvent` por nombre.
- `UAmaruAbilitySet`
  - Asset reutilizable para otorgar/remover abilities/effects/attributes en bloque.

### Flujo futuro

1. `PlayerState` replica la seleccion de loadout.
2. `LoadoutComponent` precarga y resuelve el asset.
3. `LoadoutComponent` aplica startup effects y base abilities usando handles persistentes.
4. `WeaponManagerComponent` equipa arma y gestiona solo abilities de arma.
5. UI escucha un controller/interfaz tipada.
6. `GameMode` define reglas de modo sin tocar detalles internos de GAS.

## Roadmap priorizado

### P1 - Estabilidad

- Extraer y centralizar el lifecycle de abilities/effects en handles bien administrados.
- Eliminar `LoadSynchronous` de runtime critico.
- Blindar toda operacion de grant/equip/spawn detras de autoridad de servidor.
- Sustituir bindings UI por interfaces o widget controllers.

### P2 - Escalabilidad

- Crear `AbilitySet` y `LoadoutSet` como assets dedicados.
- Separar logica de personaje en componentes de input, loadout y presentacion.
- Normalizar gameplay tags y documentarlos por dominio: input, cooldown, state, event, cue.
- Reducir debug temporal y convertirlo en logs con categorias utiles.

### P3 - Produccion

- Anadir documentacion base del proyecto y de flujo GAS.
- Crear checklist de pruebas multijugador por respawn, cambio de Inka, cambio de arma y cambio de modo.
- Anadir smoke tests manuales o automatizados para grants/removals de abilities.
- Revisar empaquetado de plugins, assets sobrantes y contenido heredado del template.
