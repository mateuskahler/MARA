/**
	UFRGS - Engenharia Elétrica
	 * 
	 * Definição de um Indutor
	 * 
	Mateus Kahler
*/

#ifndef SIMULADOR_ESTOQUE_INDUTOR_INCLUSO
#define SIMULADOR_ESTOQUE_INDUTOR_INCLUSO sim

#include <Circuito/Componente_Base.hpp>

namespace Circuito {
namespace Componentes {

struct Indutor : public Circuito::Componente
{
	NUM_T Indutancia; // Henry
	Indutor(std::string , NUMs_t parametros)
	{
		Indutancia = parametros[0];
	}
	bool modelo_ajustavel()const override{return false;}
	Circuito::Modelo_Ramificado modelo_inicial(Condicionamento::Contexto_Analise const &)override
	{
		Circuito::Modelo_Ramificado M;
		M.circuito.Adiciona(Circuito::Ramo_Base::Cria_Indutivo(Indutancia), 0, 1);
		return M;
	}
	Circuito::Modelo_Ramificado modelo_condicional(Condicionamento::Estado_Subcircuito const & , Condicionamento::Contexto_Analise const & contexto) override
	{
		return modelo_inicial(contexto);
	}
	bool possui_propriedade(std::string propriedade) override
	{
		if(( propriedade == "corrente") || 
			 ( propriedade == "i")        ||
			 ( propriedade == "ddp")      ||
			 ( propriedade == "u")          )
			{return true;}
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
		return std::string("Indutor");
	}
	std::string tipo_str()const override
	{
		return Tipo_str();
	}
};

} } //fim namespaces Circuito::Componentes

#endif //fim header guard SIMULADOR_ESTOQUE_INDUTOR_INCLUSO