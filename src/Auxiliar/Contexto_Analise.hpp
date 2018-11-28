/**
	UFRGS - Engenharia Elétrica
	 * 
	 * Este arquivo contém definição de contexto de análise.
	 * O contexto de análise inclui algumas informações que descrevem o estado atual da análise, requeridos pelos componentes para definir seu modelo equivalente.
	 * 
	Mateus Kahler
*/

#ifndef SIMULADOR_AUXILIAR_CONTEXTO_ANALISE_INCLUIDO
#define SIMULADOR_AUXILIAR_CONTEXTO_ANALISE_INCLUIDO sim

#include <Auxiliar/Dogmas.hpp>

namespace Condicionamento
{
	
struct Contexto_Analise
{
	bool quiescente;
	ind_t contagem_itera;
	NUM_T  t;
	NUM_T dt;
	static Contexto_Analise GeraContextoQuiescente()
	{
		Contexto_Analise contexto;
		contexto.quiescente = true;
		contexto.contagem_itera = 0;
		contexto.t  = 0.;
		contexto.dt = 0.;
		return contexto;
	}
	static Contexto_Analise GeraContextoTransiente(NUM_T dt)
	{
		Contexto_Analise contexto;
		contexto.quiescente = false;
		contexto.contagem_itera = 0;
		contexto.t  = 0.;
		contexto.dt = dt;
		return contexto;
	}
};

}// fim de namespace Condicionamento

#endif //fim header guard SIMULADOR_AUXILIAR_CONTEXTO_ANALISE_INCLUIDO