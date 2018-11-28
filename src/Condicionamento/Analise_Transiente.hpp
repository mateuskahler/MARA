/**
	UFRGS - Engenharia Elétrica
	 * 
	 * Este arquivo contém definição da classe que representa Análise Transiente
	 * 
	Mateus Kahler
*/

#ifndef SIMULADOR_CONDICIONAMENTO_ANALISE_TRANSIENTE_INCLUIDA
#define SIMULADOR_CONDICIONAMENTO_ANALISE_TRANSIENTE_INCLUIDA sim

#include <Condicionamento/Analise_Quiescente.hpp>
#include <Algebra/Integrador.hpp>

namespace Condicionamento {

struct Analise_Transiente : public Analise_Quiescente
{
	static constexpr ind_t max_analise_passos_de_tempo = 128*1024; //limite arbitrário de número máximo de passos para análise transiente
	//estrutura específica da Análise Transisten para associar ramo a integrador
	struct indexador_ramo_integravel_t
	{
		ind_t ramo_i;
		Algebra::Integrador integrador;
		indexador_ramo_integravel_t(ind_t Ramo_i, NUM_T parametro_integrador)
		  : ramo_i(Ramo_i), integrador(parametro_integrador){;}
	};
	//memória
	std::vector<indexador_ramo_integravel_t> lista_cap;
	std::vector<indexador_ramo_integravel_t> lista_ind;
	Condicionamento::Contexto_Analise contexto_atual;
	Tratamento::Resultados_t resultado_atual;
	NUM_T tempo_final;
	NUM_T tempo_passo;
	NUMs_t tempo_pontos;
	//construtor
	Analise_Transiente(NUM_T Tempo_total, NUM_T Tempo_passo)
	  : tempo_final(std::abs(Tempo_total)),  tempo_passo(std::abs(Tempo_passo))
	{
		
	}
	std::string Tipo_Analise()const override
	{
		return std::string("Transiente");
	}
	Tratamento::Resultados_t resolve(Condicionamento::Circuito_Esquematico const & esquema, Estoque::Estoque_t const & estoque) override
	{
		std::cout << "resolvendo analise transiente" << std::endl;
		std::cout << "tempo final : " << this->tempo_final << std::endl;
		std::cout << "tempo passo : " << this->tempo_passo << std::endl;
		gera_ponto_inicial(esquema, estoque);
		carrega_integradores_de_ponto_op();
		carrega_circuito_equivalente_de_integradores();
		transita_para_analise_transiente();
		
		acrescenta_passo_a_objetivos_da_analise(resultado_atual);
		ind_t contagem_passo_tempos = 0;
		while((contexto_atual.t < tempo_final)&&(contagem_passo_tempos < max_analise_passos_de_tempo))
		{
			contagem_passo_tempos++;
			realiza_passo_simulacao();
			acrescenta_passo_a_objetivos_da_analise(resultado_atual);
		}
		//return extrai_objetivos_da_analise();
		resultado_atual.variavel_independente = tempo_pontos_simulados();
		return resultado_atual;
	}
	NUMs_t const & tempo_pontos_simulados()
	{
		return tempo_pontos;
	}
	~Analise_Transiente() = default;
protected:
	void gera_ponto_inicial(Condicionamento::Circuito_Esquematico const & esquema, Estoque::Estoque_t const & estoque)
	{
		contexto_atual = Contexto_Analise::GeraContextoQuiescente();
		tempo_pontos.clear();
		lista_cap.clear();
		lista_ind.clear();
		//carrega esquemático para análise quiescente inicial
			carrega_esquematico(esquema, contexto_atual, estoque);
		//antes de seguir tratamento da análise quiescente, coleta ramos reativos
			for(auto chave_c_s : circuito_equivalente.m_ind_c_s)
			{
				auto ramos_com_estado_i = circuito_equivalente.circuito.RamosiDeSubcircuito_comEstado(chave_c_s.second);
				for(auto ri:ramos_com_estado_i)
				{
					auto const & ramo = circuito_equivalente.circuito.CircuitoAcesso().ramos[ri].elemento;
					if(ramo.capacitivo())
					{
						lista_cap.emplace_back(ri, NUM_T(1)/ramo.pr);
						lista_cap.back().ramo_i = ri;
					}
					else if(ramo.indutivo())
					{
						lista_ind.emplace_back(ri, NUM_T(1)/ramo.pr);
						lista_ind.back().ramo_i = ri;
					}
					else
						{ throw std::runtime_error("Analise_transiente::gera_ponto_inicial(): Ramo de tipo nao implementado requer estado inicial"); }
				}
			}
		//realiza análise quiescente para resolver ponto inicial
			substitui_ramos_ordem1_por_quiescentes();
			Estabiliza_ponto_op( contexto_atual );
			tempo_pontos.emplace_back(0.);
	}
	void carrega_integradores_de_ponto_op()
	{
		auto const & circuito = circuito_equivalente.circuito.CircuitoAcesso();
		for(auto & ci:lista_cap)
		{
			auto estado_ramo = sistema_condicionado.Extrai_Estado_de_Ramo(ponto_op, circuito, ci.ramo_i);
			ci.integrador.Y = estado_ramo.u;
		}
		for(auto & li:lista_ind)
		{
			auto estado_ramo = sistema_condicionado.Extrai_Estado_de_Ramo(ponto_op, circuito, li.ramo_i);
			li.integrador.Y = estado_ramo.i;
		}
	}
	void carrega_circuito_equivalente_de_integradores()
	{
		auto & circuito = circuito_equivalente.circuito.CircuitoAcesso();
		auto & ramos = circuito.ramos;
		for(auto & ci:lista_cap)
		{
			ramos[ci.ramo_i].elemento.tipo = Circuito::Ramo_Tipo_enum_t::DDP_Fixa;
			ramos[ci.ramo_i].elemento.pr   = ci.integrador.Y;
		}
		for(auto & ci:lista_ind)
		{
			ramos[ci.ramo_i].elemento.tipo = Circuito::Ramo_Tipo_enum_t::Corrente_Fixa;
			ramos[ci.ramo_i].elemento.pr = ci.integrador.Y;
		}
		sistema_condicionado = Sistema_ANMM(circuito);
	}
	void transita_para_analise_transiente()
	{
		//inicializa memória para resultados
		resultado_atual = Tratamento::Resultados_t();
		for(auto const obj: objetivos)
			{ resultado_atual.resultados.emplace_back(obj); }
		contexto_atual = Contexto_Analise::GeraContextoTransiente(tempo_passo);
	}
	void acrescenta_passo_a_objetivos_da_analise(Tratamento::Resultados_t & resultado_anterior)
	{
		auto const & circ_ramificado =  circuito_equivalente.circuito.CircuitoAcesso();
		ind_t resultado_pos = 0;
		for(auto const obj: objetivos)
		{
			if(obj.nodal)
			{
				auto const nodo_i = m_nodo_nome_i.find( obj.nome )->second;
				resultado_anterior.resultados[resultado_pos].valor.emplace_back( sistema_condicionado.Extrai_Estado_de_Nodo(ponto_op, nodo_i) );
			}
			else
			{
				auto const comp_i  = m_comp_nome_i.find( obj.nome )->second;
				auto const ramos_i = circuito_equivalente.circuito.RamosiDeSubcircuito(comp_i);
				auto const nodos_i = circuito_equivalente.circuito.ConexoesiDeSubcircuito(comp_i);
				auto const & estado = sistema_condicionado.Compila_Estado_Componente(ponto_op, circ_ramificado, ramos_i, nodos_i);
				resultado_anterior.resultados[resultado_pos].valor.emplace_back( circuito_componentes.Componente_Acesso(comp_i).extrai_propriedade( obj.propriedade, estado ) );
			}
			resultado_pos++;
		}
	}
	void realiza_passo_simulacao()
	{
		auto & circuito = circuito_equivalente.circuito.CircuitoAcesso();
		auto & ramos = circuito.ramos;
		//integração numérica de capacitores e indutores
		for(auto & ci:lista_cap)
		{
			auto corrente_instantanea = sistema_condicionado.Extrai_Estado_de_Ramo(ponto_op, circuito, ci.ramo_i).i;
			ci.integrador.integra_passo(corrente_instantanea, contexto_atual.dt);
			ramos[ci.ramo_i].elemento.pr = ci.integrador.Y;
		}
		for(auto & ci:lista_ind)
		{
			auto ddp_instantanea = sistema_condicionado.Extrai_Estado_de_Ramo(ponto_op, circuito, ci.ramo_i).u;
			ci.integrador.integra_passo(ddp_instantanea, contexto_atual.dt);
			ramos[ci.ramo_i].elemento.pr = ci.integrador.Y;
		}
		contexto_atual.t += contexto_atual.dt;
		contexto_atual.contagem_itera = 0;
		Estabiliza_ponto_op(contexto_atual);
		tempo_pontos.emplace_back(contexto_atual.t);
	}
};

} //fim namespace Condicionamento

#endif //fim header guard SIMULADOR_CONDICIONAMENTO_ANALISE_TRANSIENTE_INCLUIDA