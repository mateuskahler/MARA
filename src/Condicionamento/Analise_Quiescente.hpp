/**
	UFRGS - Engenharia Elétrica
	 * 
	 * Este arquivo contém definição da classe que representa Análise Quiescente
	 * 
	Mateus Kahler
*/

#ifndef SIMULADOR_CONDICIONAMENTO_ANALISE_QUIESCENTE_INCLUIDA
#define SIMULADOR_CONDICIONAMENTO_ANALISE_QUIESCENTE_INCLUIDA sim

#include <Condicionamento/Analise_Base.hpp>
#include <Condicionamento/Equacionador_ANMM.hpp>
#include <Algebra/Pivoteamento_Direto.hpp>

namespace Condicionamento {

struct Analise_Quiescente : public Analise_Base
{
	static constexpr ind_t max_analise_iteracoes_para_estabilizar = 1000;
	//memória
	Condicionamento::Sistema_ANMM sistema_condicionado;
	NUMs_t ponto_op; //vetor resultado do sistema linear atrelado ao circuito equivalente
	~Analise_Quiescente()=default;
	virtual std::string Tipo_Analise()const override
	{
		return std::string("Quiescente");
	}
	virtual Tratamento::Resultados_t resolve(Condicionamento::Circuito_Esquematico const & esquema, Estoque::Estoque_t const & estoque) override
	{
		auto contexto = Contexto_Analise::GeraContextoQuiescente();
		//expande componentes em circuito equivalente
			carrega_esquematico(esquema, contexto, estoque);
		//faz substituições próprias da análise quiescente
			substitui_ramos_ordem1_por_quiescentes();
		//estabiliza ponto de operação
			Estabiliza_ponto_op(contexto);
		return extrai_objetivos_da_analise();
	}
protected:
	void Estabiliza_ponto_op(Condicionamento::Contexto_Analise & contexto)
	{
	//resolve e estabiliza circuito
		sistema_condicionado = Condicionamento::Sistema_ANMM(circuito_equivalente.circuito.CircuitoAcesso());
		ponto_op = Algebra::Resolve( sistema_condicionado.sistema_linear.A, sistema_condicionado.sistema_linear.y);
		bool estavel = false;
		for(; contexto.contagem_itera<max_analise_iteracoes_para_estabilizar; contexto.contagem_itera++)
		{
			//se não for requerida nenhuma atualização significativa, interrompe
			if(!itera_estabilizar_ponto_op(contexto, ponto_op))
				{ estavel = true; break; };
			sistema_condicionado = Condicionamento::Sistema_ANMM(circuito_equivalente.circuito.CircuitoAcesso());
			ponto_op = Algebra::Resolve( sistema_condicionado.sistema_linear.A, sistema_condicionado.sistema_linear.y);
		}
		if(!estavel)
			{ throw std::runtime_error("Analise_Quiescente::resolve() : Circuito instavel dentro dos limites do simulador");}
	}
	bool itera_estabilizar_ponto_op(Contexto_Analise const contexto, NUMs_t const & ponto_inicial)
	{
		bool algum_componente_atualizado = false;
		auto const & comp_ajustaveis_is = circuito_componentes.ContiguoAcesso_Ajustaveis();
		auto const & circ_ramificado =  circuito_equivalente.circuito.CircuitoAcesso();
		for(ind_t comp_a_i : comp_ajustaveis_is)
		{
			auto const & ramos_i = circuito_equivalente.circuito.RamosiDeSubcircuito(comp_a_i);
			auto const & nodos_i = circuito_equivalente.circuito.ConexoesiDeSubcircuito(comp_a_i);
			auto const & estado = sistema_condicionado.Compila_Estado_Componente(ponto_inicial, circ_ramificado, ramos_i, nodos_i);
			auto novo_modelo = circuito_componentes.Componente_Acesso(comp_a_i).modelo_condicional(estado, contexto);
			ind_t subc_a_i = circuito_equivalente.m_ind_c_s[comp_a_i];
			algum_componente_atualizado = algum_componente_atualizado || (atualiza_modelo_se_instavel(novo_modelo, subc_a_i));
		}
		return algum_componente_atualizado;
	}
	Tratamento::Resultados_t extrai_objetivos_da_analise()
	{
		Tratamento::Resultados_t resultado;
		auto const & circ_ramificado =  circuito_equivalente.circuito.CircuitoAcesso();
		for(auto const obj: objetivos)
		{
			if(obj.nodal)
			{
				auto const nodo_i = m_nodo_nome_i.find( obj.nome )->second;
				Tratamento::Resultado R(obj);
				R.valor.emplace_back( sistema_condicionado.Extrai_Estado_de_Nodo(ponto_op, nodo_i) );
				resultado.resultados.emplace_back( R );
			}
			else
			{
				auto const comp_i = m_comp_nome_i.find( obj.nome )->second;
				auto const ramos_i = circuito_equivalente.circuito.RamosiDeSubcircuito(comp_i);
				auto const nodos_i = circuito_equivalente.circuito.ConexoesiDeSubcircuito(comp_i);
				auto const & estado = sistema_condicionado.Compila_Estado_Componente(ponto_op, circ_ramificado, ramos_i, nodos_i);
				Tratamento::Resultado R(obj);
				R.valor.emplace_back(circuito_componentes.Componente_Acesso(comp_i).extrai_propriedade( obj.propriedade, estado ));
				resultado.resultados.emplace_back( R );
			}
		}
		return resultado;
	}
	void substitui_ramos_ordem1_por_quiescentes()
	{
		//mapeia componentes com seus ramos (indexados no circuito equivalente) que requerem estado inicial
			std::map<ind_t, inds_t> m_estados_requeridos_c_rii;
			for(auto chave_c_s : circuito_equivalente.m_ind_c_s)
			{
				m_estados_requeridos_c_rii[chave_c_s.first] = circuito_equivalente.circuito.RamosiDeSubcircuito_comEstado(chave_c_s.second);
			}
		//subtitui todos os ramos de ordem 1 por equivalente, explícito ou implícito
			auto fornecidos_m_cit = circuito_equivalente.estados_fornecidos.begin();
			auto requeridos_m_cit = m_estados_requeridos_c_rii.begin();
			ind_t const NC_req = m_estados_requeridos_c_rii.size();
			ind_t pos_req_comp = 0;
			ind_t pos_for_comp = 0;
			while(pos_req_comp<NC_req)
			{
				ind_t const comp_i = requeridos_m_cit->first;
				ind_t const NR_req    = requeridos_m_cit->second.size();
				auto requeridos_m_rit = requeridos_m_cit->second.begin();
				ind_t pos_req_ramo = 0;
				//primeiro, verifica se há estados fornecidos para este componente e substitui por ramo equivalente
					if(fornecidos_m_cit!=circuito_equivalente.estados_fornecidos.end())
					{
						ind_t comp_fornecido_i = fornecidos_m_cit->first;
						//verifica se estados são fornecidos para este componente
						if(comp_fornecido_i == comp_i)
						{
							ind_t const NR_for = fornecidos_m_cit->second.size();
							auto fornecidos_m_rit = fornecidos_m_cit->second.begin();
							ind_t pos_for_ramo = 0;
							while(pos_for_ramo < NR_for)
							{
								if(pos_req_ramo >= NR_req)
									{	throw std::runtime_error("Fornecidas condicoes iniciais em excesso para componente");	}
								NUM_T valor = *fornecidos_m_rit;
								ind_t ramo_ii = *requeridos_m_rit;
								if(circuito_equivalente.circuito.CircuitoAcesso().ramos[ramo_ii].elemento.capacitivo())
								{
									auto ramo_equivalente = Circuito::Ramo_Base::Cria_DDP_Fixa(valor);
									circuito_equivalente.circuito.CircuitoAcesso().SubstituiRamo(ramo_ii, ramo_equivalente);
								}			
								else if(circuito_equivalente.circuito.CircuitoAcesso().ramos[ramo_ii].elemento.indutivo())
								{
									auto ramo_equivalente = Circuito::Ramo_Base::Cria_Corrente_Fixa(valor);
									circuito_equivalente.circuito.CircuitoAcesso().SubstituiRamo(ramo_ii, ramo_equivalente);
								}
								else
									{ throw std::runtime_error("Ramo de tipo nao implementado requer estado inicial");	}
								fornecidos_m_rit++;
								requeridos_m_rit++;
								pos_req_ramo++;
								pos_for_ramo++;
							}
							//avança para próximo componente que possui condições fornecidas
							pos_for_comp++;
							fornecidos_m_cit++;
						}
					}
				//subtitui ramos sem estado inicial por ramo equivalente de derivada nula implícita
					while(pos_req_ramo<NR_req)
					{
						ind_t ramo_ii = *requeridos_m_rit;
						if(circuito_equivalente.circuito.CircuitoAcesso().ramos[ramo_ii].elemento.capacitivo())
						{
							auto ramo_equivalente = Circuito::Ramo_Base::Cria_Corrente_Fixa(0);
							circuito_equivalente.circuito.CircuitoAcesso().SubstituiRamo(ramo_ii, ramo_equivalente);
						}			
						else if(circuito_equivalente.circuito.CircuitoAcesso().ramos[ramo_ii].elemento.indutivo())
						{
							auto ramo_equivalente = Circuito::Ramo_Base::Cria_DDP_Fixa(0);
							circuito_equivalente.circuito.CircuitoAcesso().SubstituiRamo(ramo_ii, ramo_equivalente);
						}
						else
							{ throw std::runtime_error("Ramo de tipo nao implementado requer estado inicial");	}
						requeridos_m_rit++;
						pos_req_ramo++;
					}
				//avança para próximo componente
				pos_req_comp++;
				requeridos_m_cit++;
			}
	}
};

} //fim namespace Condicionamento

#endif //fim header guard SIMULADOR_CONDICIONAMENTO_ANALISE_QUIESCENTE_INCLUIDA