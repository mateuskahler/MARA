/**
	UFRGS - Engenharia Elétrica
	 * 
	 * Definição de uma fonte de tensão senoidal
	 * 
	Mateus Kahler
*/

#ifndef SIMULADOR_ESTOQUE_FONTEVSENO_INCLUSA
#define SIMULADOR_ESTOQUE_FONTEVSENO_INCLUSA sim

#include <Circuito/Componente_Base.hpp>

namespace Circuito {
namespace Componentes {

struct FonteVSeno : public Circuito::Componente
{
	ind_t modelo_id;
	NUM_T DDP_pico;  // Volts
	NUM_T R_interna; // Ohms (para modelo que possui resistência interna)
	NUM_T freq_seno; //frequência de oscilação da diferença de potencial
	// parâmetros : ddp (pico), freq(Hz), R_interna(Ohm)
	FonteVSeno(std::string modelo, NUMs_t parametros)
	{
		if(modelo == u8"resistênciaInterna")
		{
			modelo_id = 1;
			R_interna = parametros[2];
		}
		else
		{
			modelo_id = 0;
		}
		DDP_pico = parametros[0];
		freq_seno = parametros[1];
	}
	bool modelo_ajustavel()const override{return true;}
	Circuito::Modelo_Ramificado modelo_inicial(Condicionamento::Contexto_Analise const & )override
	{
		Circuito::Modelo_Ramificado M;
		if(modelo_id == 1)
		{
			M.circuito.Adiciona(Circuito::Ramo_Base::Cria_DDP_Fixa(0.), 0, 2);
			M.circuito.Adiciona(Circuito::Ramo_Base::Cria_Resistivo(R_interna), 2, 1);
		}
		else
		{
			M.circuito.Adiciona(Circuito::Ramo_Base::Cria_DDP_Fixa(0.), 0, 1);
		}
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
			NUM_T constexpr pi_ = std::acos(-1);
			NUM_T Vt = DDP_pico*std::sin(contexto.t*2*pi_*freq_seno);
			std::cout << "t = "  << contexto.t << " , " <<  Vt << std::endl;
			Circuito::Modelo_Ramificado M;
			if(modelo_id == 1)
			{
				M.circuito.Adiciona(Circuito::Ramo_Base::Cria_DDP_Fixa(Vt), 0, 2);
				M.circuito.Adiciona(Circuito::Ramo_Base::Cria_Resistivo(R_interna), 2, 1);
			}
			else
			{
				M.circuito.Adiciona(Circuito::Ramo_Base::Cria_DDP_Fixa(Vt), 0, 1);
			}
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
		return std::string("FonteDDPSeno");
	}
	std::string tipo_str()const override
	{
		return Tipo_str();
	}
};

} } //fim namespaces Circuito::Componentes

#endif //fim header guard SIMULADOR_ESTOQUE_FONTEVSENO_INCLUSA