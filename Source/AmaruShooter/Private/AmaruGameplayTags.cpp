#include "AmaruGameplayTags.h"

namespace AmaruTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Event_Death, "Event.Death", "Disparado una vez por el AttributeSet al recibir daño letal");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Event_Kill, "Event.Kill", "Enviado al ASC del asesino cuando confirma una kill");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Dead, "State.Dead", "El jugador está muerto; bloquea activación de habilidades");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Stealth, "State.Stealth", "Invisible (Paso del Atoq)");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Focus, "State.Focus", "Precisión mejorada (Ojo del Centinela)");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Rage, "State.Rage", "Furia del Yawar: más daño/cadencia, cura al matar");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Protected, "State.Protected", "Reducción de daño (domo de Pachamama, dash de Wayra)");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Haste, "State.Haste", "Velocidad de ataque aumentada (Danza de los Apus)");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Slowed, "State.Slowed", "Ralentizado (enemigos dentro del domo)");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Data_Damage, "Data.Damage", "SetByCaller: magnitud de daño");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Data_Cooldown, "Data.Cooldown", "SetByCaller: duración de cooldown");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Data_Heal, "Data.Heal", "SetByCaller: magnitud de curación");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Data_Shield, "Data.Shield", "SetByCaller: puntos de escudo otorgados");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Cooldown_Ultimate, "Cooldown.Ultimate", "Cooldown corto anti-doble-activación de ultimates");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Cooldown_Atoq_Primary, "Cooldown.Atoq.Primary", "Cadencia de la Ballesta de Hierro");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Cooldown_Atoq_Ability1, "Cooldown.Atoq.Ability1", "Cooldown de Ojo del Centinela");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Cooldown_Atoq_Ability2, "Cooldown.Atoq.Ability2", "Cooldown de Paso del Atoq");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Cooldown_Pachamama_Primary, "Cooldown.Pachamama.Primary", "Cadencia del Núcleo de Arcilla");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Cooldown_Pachamama_Ability1, "Cooldown.Pachamama.Ability1", "Cooldown de Abrazo de la Tierra");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Cooldown_Pachamama_Ability2, "Cooldown.Pachamama.Ability2", "Cooldown de Muro de la Pachamama");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Cooldown_Wayra_Primary, "Cooldown.Wayra.Primary", "Cadencia del Bastón Ritual");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Cooldown_Wayra_Ability1, "Cooldown.Wayra.Ability1", "Cooldown de Sanación del Viento (reducible al acertar orbes)");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Cooldown_Wayra_Ability2, "Cooldown.Wayra.Ability2", "Cooldown de Paso del Viento");
}
