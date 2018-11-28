/**
	UFRGS - Engenharia Elétrica
	 * 
	 * Definição de um Diodo
	 * 
	Mateus Kahler
*/

#ifndef SIMULADOR_ESTOQUE_DIODO_INCLUSO
#define SIMULADOR_ESTOQUE_DIODO_INCLUSO sim

#include <Circuito/Componente_Base.hpp>
#include <Auxiliar/Jogral.hpp>

namespace Circuito {
namespace Componentes {

struct Diodo : public Circuito::Componente
{
	static constexpr NUM_T V_avalanche_direta_ideal      = 0;
	static constexpr NUM_T R_equivalente_bloqueado_ideal = 10e6;  //bloqueio de 10MOhm
	static constexpr NUM_T I_saturacao_direta_ideal      =  1e3;  //corrente de 1kA
	static constexpr NUM_T V_margem_idealidade_ideal     = 50e-3; //margem de 50mV
	static constexpr NUM_T R_equivalente_inicial_ideal   = 1;
	
	ind_t modelo_id;
	//propriedades para modelagem idealizada
		NUM_T V_avalanche_direta;
		NUM_T R_equiv_bloqueio;
		NUM_T I_saturado_direto;
		NUM_T V_margem_idealidade;
	//propriedades extras para modelagem exponencial
		
	//estados para simulação/estabilização
		NUM_T R_equiv_atual;
		Auxiliar::Jogral iterador_estabilidade;
	
	Diodo(std::string modelo, NUMs_t parametros)
		: modelo_id(0),
			V_avalanche_direta(V_avalanche_direta_ideal), 
			R_equiv_bloqueio(R_equivalente_bloqueado_ideal),
			I_saturado_direto(I_saturacao_direta_ideal),
			V_margem_idealidade(V_margem_idealidade_ideal), 
			R_equiv_atual(R_equivalente_inicial_ideal)
	{
		if(modelo == "ideal")
		{
			if(parametros.size() == 4)
			{
				modelo_id = 0;
				V_avalanche_direta  = parametros[0];
				R_equiv_bloqueio    = parametros[1];
				I_saturado_direto   = parametros[2];
				V_margem_idealidade = parametros[3];
			}
		}
		if(modelo == "ideal1")
		{
			if(parametros.size() == 1)
			{
				modelo_id = 0;
				V_avalanche_direta  = parametros[0];
			}
		}
	}
	bool modelo_ajustavel()const override{return true;}
	Circuito::Modelo_Ramificado modelo_inicial(Condicionamento::Contexto_Analise const &)override
	{
		Circuito::Modelo_Ramificado M;
		M.circuito.Adiciona(Circuito::Ramo_Base::Cria_Resistivo(R_equivalente_inicial_ideal), 0, 1);
		return M;
	}
public:
	Circuito::Modelo_Ramificado modelo_condicional(Condicionamento::Estado_Subcircuito const & estado, Condicionamento::Contexto_Analise const & contexto) override
	{
		switch(modelo_id)
		{
			//modelo ideal
			case(0):
				{ return modelo_condicional_idealizado(estado, contexto); }
			//modelo baseado na equação de Shockley
			case(1):
			{
				throw std::runtime_error("Diodo::modelo_condicional(): Estou shockleado");
			}
			break;
			default:
				throw std::runtime_error("Diodo::modelo_condicional(): Requisitado modelo inexistente de Diodo");
		}
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
	std::string tipo_str()const override
	{
		return std::string("Diodo");
	}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// métodos de estabilização para cada modelo do dioso
	Circuito::Modelo_Ramificado modelo_condicional_idealizado(
		Condicionamento::Estado_Subcircuito const & estado, 
		Condicionamento::Contexto_Analise const & )
	{
		NUM_T const u = estado.e_ramos[0].u;
		NUM_T const i = estado.e_ramos[0].i;
		NUM_T const limite_inferior = V_avalanche_direta;
		NUM_T const limite_superior = V_avalanche_direta + V_margem_idealidade;
		NUM_T const Isup = I_saturado_direto;
		NUM_T const Iinf = (V_avalanche_direta)/R_equiv_bloqueio;
		NUM_T const R_equiv_ativo = limite_superior/I_saturado_direto;
		bool R_passo_sobe;
		if( u <= limite_inferior )
			{	 
				R_passo_sobe = true;
			}
		else if ( u >= limite_superior )
			{ 
				R_passo_sobe = false; 
			}
		else
			{
				NUM_T i_esperada = (u-V_avalanche_direta);
					i_esperada *= (Isup - Iinf);
					i_esperada /= (V_margem_idealidade);
					i_esperada += Iinf;
				R_passo_sobe = (i > i_esperada);
			}
		if( R_passo_sobe )
			{	iterador_estabilidade.passo_sobe();	}
		else
			{	iterador_estabilidade.passo_desce(); }
		NUM_T interpolador = iterador_estabilidade.valor_atual();
		NUM_T R_interpolada = R_equiv_bloqueio*(interpolador) + R_equiv_ativo*(NUM_T(1)-interpolador);
		R_equiv_atual = R_interpolada;
		Circuito::Modelo_Ramificado M;
		M.circuito.Adiciona(Circuito::Ramo_Base::Cria_Resistivo(R_equiv_atual), 0, 1);
		return M;
	}
};

} } //fim namespaces Circuito::Componentes

#endif //fim header guard SIMULADOR_ESTOQUE_DIODO_INCLUSO