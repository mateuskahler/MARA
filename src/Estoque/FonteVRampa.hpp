/**
	UFRGS - Engenharia Elétrica
	 * 
	 * Definição de uma fonte de tensão fixa
	 * 
	Mateus Kahler
*/

#ifndef SIMULADOR_ESTOQUE_FONTEVRAMPA_INCLUSA
#define SIMULADOR_ESTOQUE_FONTEVRAMPA_INCLUSA sim

#include <Circuito/Componente_Base.hpp>

namespace Circuito {
namespace Componentes {

struct FonteVRampa : public Circuito::Componente
{
	ind_t modelo_id;
	NUM_T DDP_0;  // [V]olts
	NUM_T DDP_1;  // [V]olts
	NUM_T dT;     // [s]egundos
	// parâmetros : ddp0[V], ddp1[V], tempo_subida(s)
	FonteVRampa(std::string modelo, NUMs_t parametros)
	{
		modelo_id = 0;
		DDP_0 = parametros[0];
		DDP_1 = parametros[1];
		dT    = parametros[2];
	}
	bool modelo_ajustavel()const override{return true;}
	Circuito::Modelo_Ramificado modelo_inicial(Condicionamento::Contexto_Analise const & )override
	{
		Circuito::Modelo_Ramificado M;
		M.circuito.Adiciona(Circuito::Ramo_Base::Cria_DDP_Fixa(DDP_0), 0, 1);
		return M;
	}
	Circuito::Modelo_Ramificado modelo_condicional(Condicionamento::Estado_Subcircuito const & , Condicionamento::Contexto_Analise const & contexto) override
	{
		if(contexto.quiescente)
		{
			return modelo_inicial(contexto);
		}
		else
		{
			NUM_T const t = contexto.t;
			NUM_T Vt;
			if(t >= dT)
			{
				Vt = DDP_1;
			}
			else
			{
				// V(t) = V(0) + dV/dt * (t - t0)
				NUM_T const dVdt = (DDP_1-DDP_0)/dT;
				Vt = DDP_0 + t*dVdt;
			}
			Circuito::Modelo_Ramificado M;
			M.circuito.Adiciona(Circuito::Ramo_Base::Cria_DDP_Fixa(Vt), 0, 1);
			return M;
		}
	}
	bool possui_propriedade(std::string propriedade) override
	{
		if(( propriedade == "corrente") || 
			 ( propriedade == "i")      ||
			 ( propriedade == "ddp")    ||
			 ( propriedade == "u")        )
			{ return true; }
		return false;
	}
	NUM_T extrai_propriedade(std::string propriedade, Condicionamento::Estado_Subcircuito const & estado) override
	{
		if(( propriedade == "corrente") || 
			 ( propriedade == "i")           )
		{
			return estado.e_ramos[0].i;
		}
		 if(( propriedade == "ddp") ||
			  ( propriedade == "u")     )
		{
			return estado.potenciais_conexoes[0]-estado.potenciais_conexoes[1];
		}
		return static_cast<NUM_T>(0);
	}
	ind_t Conexoes_quantidade()const override
	{
		return 2;
	}
	static std::string Tipo_str()
	{
		return std::string("FonteDDPRampa");
	}
	std::string tipo_str()const override
	{
		return Tipo_str();
	}
};

} } //fim namespaces Circuito::Componentes

#endif //fim header guard SIMULADOR_ESTOQUE_FONTEVRAMPA_INCLUSA