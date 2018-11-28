 /**
	UFRGS - Engenharia Elétrica
	 * 
	 * Circuito composto por subcircuitos
	 * 
	Mateus Kahler
*/

#ifndef SIMULADOR_CIRCUITO_CIRCUITO_DE_COMPONENTES_INCLUSO
#define SIMULADOR_CIRCUITO_CIRCUITO_DE_COMPONENTES_INCLUSO sim

#include <Circuito/Circuito_de_Subcircuitos.hpp>
#include <Circuito/Componente_Base.hpp>

namespace Circuito {
	
struct Circuito_Equivalente_t
{
	Circuito_de_Subcircuitos  circuito; //circuito de subcircuitos que representam componentes
	std::map<ind_t, ind_t>  m_ind_c_s;  //mapa de índice de componente para subcircuito
	std::vector<referencia_de_controle_t> controles_requeridos;
	std::map<ind_t, NUMs_t> estados_fornecidos;
};

struct Circuito_de_Componentes
{
	typedef std::unique_ptr<Componente> comp_upt_t;
	typedef VetorIndexado<Conectado<comp_upt_t>>  veci_comps_conectados_t;
	//construtor padrão
		Circuito_de_Componentes() = default;
	// adiciona componente fornecendo conexões e componente encapsulado em ponteiro único (std::unique_ptr)
		ind_t AdicionaComponente(inds_t conexoes, comp_upt_t componente_unico)
		{
			Conectado<comp_upt_t> comp_conectado( std::move(componente_unico), std::move(conexoes) );
			return componentes_conectados.Adiciona(std::move(comp_conectado));			
		}
	// adiciona componente fornecendo conexões, nome de modelo e parâmetros para construção de componente
		template<typename COMP_T, typename ... COMP_PARAMS_T>
		ind_t AdicionaComponente(inds_t conexoes, std::string modelo_string, COMP_PARAMS_T && ... comp_parametros)
		{
			NUMs_t parametros{static_cast<NUM_T>(comp_parametros)...};
			Conectado<comp_upt_t> comp_conectado(
				comp_upt_t(new COMP_T(modelo_string, parametros)),
				std::move(conexoes) );
			return componentes_conectados.Adiciona(std::move(comp_conectado));
		}
	//lista ordenada de todos os índices associados a componentes conectados
		inds_t const & ContiguoAcesso()const
		{
			return componentes_conectados.ContiguoAcesso();
		}
	//lista ordenada de todos os índices associados a componentes conectados ajustáveis
		inds_t ContiguoAcesso_Ajustaveis()const
		{
			inds_t c_ajust_is;
			auto const & c_is = ContiguoAcesso();
			for(auto c_i:c_is)
			{
				if(componentes_conectados[c_i].elemento->modelo_ajustavel())
					{ c_ajust_is.emplace_back(c_i); }
			}
			return c_ajust_is;
		}
	//acesso a componente específico
		Componente & Componente_Acesso(ind_t comp_i)
			{	return *(componentes_conectados[comp_i].elemento);	}
		Componente const & Componente_Acesso(ind_t comp_i)const
			{	return *(componentes_conectados[comp_i].elemento);	}
	//acesso a conexões de componente específico
		inds_t & Conexoes_Acesso(ind_t comp_i)
			{	return (componentes_conectados[comp_i].conexoes);	}
		inds_t const & Conexoes_Acesso(ind_t comp_i)const
			{	return (componentes_conectados[comp_i].conexoes);	}
	//gera circuito que equivale a componentes carregados e suas conexões
	Circuito_Equivalente_t GeraCircuitoInicial(Condicionamento::Contexto_Analise contexto_montagem)
	{
		Circuito_Equivalente_t circuito_eq;
		auto const & comp_is = componentes_conectados.ContiguoAcesso();
		//conecta circuito equivalente de cada componente
		for(auto comp_i : comp_is)
		{
			Componente & comp = Componente_Acesso(comp_i);
			inds_t     & cnxs = Conexoes_Acesso(comp_i);
			auto modelo = comp.modelo_inicial(contexto_montagem);
			ind_t const sc_i = circuito_eq.circuito.Adiciona(modelo.circuito, cnxs);
			circuito_eq.m_ind_c_s[comp_i] = sc_i;
			//salva referência a ramos que requerem controle
			auto const & ramos_iis =  circuito_eq.circuito.RamosiDeSubcircuito(sc_i);
			ind_t const ramos_N = ramos_iis.size();
			for(ind_t ramo_k=0; ramo_k<ramos_N; ramo_k++)
			{
				ind_t const ramo_ii = ramos_iis[ramo_k];
				auto const & ramo = circuito_eq.circuito.CircuitoAcesso().ramos[ramo_ii].elemento;
				if(ramo.controlado())
				{
					referencia_de_controle_t ref_cont;
					ref_cont.controles_i = ind2_t{comp_i, ramo_k};
					ref_cont.corrente = ramo.controlado_por_corrente();
					circuito_eq.controles_requeridos.emplace_back(ref_cont);
				}
			}
		}
		return circuito_eq;
	}
protected:
	//memória
	veci_comps_conectados_t componentes_conectados; //vetor indexado de todos os componentes conectados
///////////////////////////////////
};

}//fim namespace Circuito


#endif // fim header guard SIMULADOR_CIRCUITO_CIRCUITO_DE_COMPONENTES_INCLUSO