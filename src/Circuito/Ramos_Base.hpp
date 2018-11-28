 /**
	UFRGS - Engenharia Elétrica
	 * 
	 * Ramos básicos.
	 *   Resistivo
	 *   Fonte de Diferença de Potencial (fixa e controlada)
	 *   Fonte de Corrente (fixa e controlada)
	 *   Capacitivo
	 *   Indutivo
	 * 
	Mateus Kahler
*/

#ifndef CIRCUITO_RAMOS_BASE_INCLUIDO
#define CIRCUITO_RAMOS_BASE_INCLUIDO sim

#include <Auxiliar/Dogmas.hpp>

namespace Circuito{

enum class Ramo_Tipo_enum_t : ind_t {
		Resistivo,
		DDP_Fixa,
		Corrente_Fixa,
		DDP_C_DDP,
		DDP_C_Corrente,
		Corrente_C_DDP,
		Corrente_C_Corrente,
		Capacitivo,
		Indutivo
	};

struct Ramo_Base
{
	Ramo_Tipo_enum_t tipo; 
	NUM_T pr; //parâmetro
	ind_t  cp, cn; //índices de controle
	Ramo_Base() = default ;
	bool controlado_por_corrente()const
	{
		return ((tipo==Ramo_Tipo_enum_t::DDP_C_Corrente) || (tipo==Ramo_Tipo_enum_t::Corrente_C_Corrente));
	}
	bool controlado_por_ddp()const
	{
		return ((tipo==Ramo_Tipo_enum_t::DDP_C_DDP) || (tipo==Ramo_Tipo_enum_t::Corrente_C_DDP));
	}
	bool controlado()const
	{
		return (controlado_por_corrente() || controlado_por_ddp());
	}
	bool capacitivo()const
	{
		return (tipo==Ramo_Tipo_enum_t::Capacitivo);
	}
	bool indutivo()const
	{
		return (tipo==Ramo_Tipo_enum_t::Indutivo);
	}
	bool ordem1()const
	{
		return (capacitivo() || indutivo());
	}
	void conecta_controle(ind_t NCp, ind_t NCn)
	{
		cp = NCp;
		cn = NCn;
	}
	static Ramo_Base Cria_Resistivo(NUM_T R)
	{
		Ramo_Base r;
		r.tipo = Ramo_Tipo_enum_t::Resistivo;
		r.pr = R;
		return r;
	}
	static Ramo_Base Cria_Capacitivo(NUM_T capacitancia)
	{
		Ramo_Base r;
		r.tipo = Ramo_Tipo_enum_t::Capacitivo;
		r.pr = capacitancia;
		return r;
	}
	static Ramo_Base Cria_Indutivo(NUM_T indutancia)
	{
		Ramo_Base r;
		r.tipo = Ramo_Tipo_enum_t::Indutivo;
		r.pr = indutancia;
		return r;
	}
	static Ramo_Base Cria_DDP_Fixa(NUM_T V)
	{
		Ramo_Base r;
		r.tipo = Ramo_Tipo_enum_t::DDP_Fixa;
		r.pr = V;
		return r;
	}
	static Ramo_Base Cria_Corrente_Fixa(NUM_T I)
	{
		Ramo_Base r;
		r.tipo = Ramo_Tipo_enum_t::Corrente_Fixa;
		r.pr = I;
		return r;
	}
	static Ramo_Base Cria_DDP_Controlada_DDP(NUM_T alfa)
	{
		Ramo_Base r;
		r.tipo = Ramo_Tipo_enum_t::DDP_C_DDP;
		r.pr = alfa;
		return r;
	}
	static Ramo_Base Cria_DDP_Controlada_Corrente(NUM_T alfa)
	{
		Ramo_Base r;
		r.tipo = Ramo_Tipo_enum_t::DDP_C_Corrente;
		r.pr = alfa;
		return r;
	}
	static Ramo_Base Cria_Corrente_Controlada_DDP(NUM_T alfa)
	{
		Ramo_Base r;
		r.tipo = Ramo_Tipo_enum_t::Corrente_C_DDP;
		r.pr = alfa;
		return r;
	}
	static Ramo_Base Cria_Corrente_Controlada_Corrente(NUM_T alfa)
	{
		Ramo_Base r;
		r.tipo = Ramo_Tipo_enum_t::Corrente_C_Corrente;
		r.pr = alfa;
		return r;
	}
};

struct Estado_Ramo
{
	NUM_T i;
	NUM_T u;
};

} //fim namespace Circuito


#endif //fim header guard CIRCUITO_RAMOS_BASE_INCLUIDO