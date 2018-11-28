/**
	UFRGS - Engenharia Elétrica
	 * 
	 * Definição de um interpretador de texto como componentes da simulação
	 * Denominado IVO: Interpretador de Verbetes como Objetos Elétricos
	 * 
	 * Os métodos implementados por este interpretador foram projetados para atender aos 
	 * pré-requisitos para implementação de um interpretador de arquivos netlist.
	 * 
	Mateus Kahler
*/

#ifndef SIMULADOR_LEITOR_IVO_INCLUSO
#define SIMULADOR_LEITOR_IVO_INCLUSO sim

#include <Auxiliar/Dogmas.hpp>
#include <Condicionamento/Analise_Quiescente.hpp>
#include <Condicionamento/Analise_Transiente.hpp>
#include <Estoque/Estoque.hpp>
#include <fstream>

namespace IVONetList
{
	enum class classe_verbete_t : ind_t
	{
		circuito,
		analise,
		componente,
		objetivo,
		comando
	};
	struct Verbete_t
	{
		ind_t linha_origem;
		classe_verbete_t tipo;
		Verbete_t (ind_t linha_n, classe_verbete_t Tipo)
		 : linha_origem(linha_n), tipo(Tipo) {;}
	};
	struct Verbete_Circuito_t : public Verbete_t
	{
		std::string Nome;
		Verbete_Circuito_t(ind_t linha_n)
		 : Verbete_t(linha_n, classe_verbete_t::circuito) {}
	};
	struct Verbete_Analise_t : public Verbete_t
	{
		enum class classe_t : ind_t {Quiescente, Transiente};
		classe_t tipo;
		NUMs_t parametros;
		static Verbete_Analise_t cria_quiescente(ind_t linha_N)
		{
			NUMs_t nenhum_parametro;
			return Verbete_Analise_t(linha_N, classe_t::Quiescente, nenhum_parametro);
		}
		static Verbete_Analise_t cria_transiente(ind_t linha_N, NUMs_t Parametros)
		{
			return Verbete_Analise_t(linha_N, classe_t::Transiente, Parametros);
		}
	protected:
		Verbete_Analise_t(ind_t linha_N, classe_t Tipo, NUMs_t Parametros)
		 : Verbete_t(linha_N, classe_verbete_t::analise),
			tipo(Tipo), parametros(Parametros)
		{ ; }
	};
	struct Verbete_Componente_t : public Verbete_t
	{
		std::string tipo;
		std::string modelo;
		std::string nome;
		NUMs_t parametros;
		NUMs_t condicoes_iniciais;
		std::vector<std::string> conexoes;
		Verbete_Componente_t(ind_t linha_N, std::string Tipo, std::string Modelo, std::string Nome, NUMs_t params)
		 : Verbete_t(linha_N, classe_verbete_t::componente), tipo(Tipo), modelo(Modelo), nome(Nome), parametros(params) { ; }
	};
	struct Verbete_Objetivo : public Verbete_t
	{
		std::string nome;
		std::string propriedade;
		Verbete_Objetivo(ind_t linha_N, std::string Nome, std::string Propriedade)
		 : Verbete_t(linha_N, classe_verbete_t::objetivo), nome(Nome), propriedade(Propriedade) { ; }
	};
	//estrutura que guarda interpretação completa de netlist
	struct ArquivoNetList
	{
		struct analise_e_objetivos_t
		{
			Verbete_Analise_t analise;
			analise_e_objetivos_t(Verbete_Analise_t Analise)
			  : analise(Analise){;}
			std::vector<Verbete_Objetivo> objetivos;
		};
//memória
		Verbete_Circuito_t verbete_circuito;
		std::vector<Verbete_Componente_t> verbetes_componentes;
		std::vector<analise_e_objetivos_t> analises;
		std::set<std::string> nodos_declarados;
		std::vector<std::vector<std::string>> nodos_agrupados;
		bool usa_virgula_como_separador_decimal;
		bool deve_usar_gnuplot;
		ArquivoNetList()
		 : verbete_circuito(-1), usa_virgula_como_separador_decimal(true), deve_usar_gnuplot(false) {;}
///métodos auxiliares para interpretação de texto
		static std::string removido_texto_pos_cerquilha(std::string Linha)
		{
			std::string nova_linha(Linha.begin(), std::find(Linha.begin(), Linha.end(), '#'));
			return nova_linha;
		}
		static bool verifica_linha_vazia(std::string Linha)
		{
			return (primeiro_char_it(Linha)==Linha.end());
		}
		static bool verifica_linha_nova_parte(std::string linha, std::string termo)
		{
			if(!(primeiro_char(linha)=='+'))
				{ return false;}
			auto pos_it = primeiro_char_it(linha);
			std::string sub_linha(pos_it+1, linha.end());
			return (primeira_palavra(sub_linha)==termo);
		}
		static bool verifica_linha_novo_comando(std::string Linha)
		{
			return (primeiro_char(Linha)=='.');
		}
		bool possui_componente_nomeado(std::string nome)
		{
			auto procura = std::find_if(verbetes_componentes.begin(), verbetes_componentes.end(), 
				[nome](Verbete_Componente_t const & comp)->bool{return comp.nome==nome;} );
			return (procura != verbetes_componentes.end());
		}
		void inicia_novo_circuito(std::string Linha, ind_t linha_N)
		{
			static const std::string termo("Circuito");
			auto pos_procura = Linha.find(termo, 0);
			if(pos_procura == termo.npos)
			{
				std::stringstream ss;
				ss << "Erro ao tentar iniciar novo circuito com linha invalida. ";
				ss <<  "Linha " << linha_N << ":" << std::endl << Linha;
				throw std::runtime_error(ss.str());
			}
			pos_procura += termo.length();
			if(pos_procura >= Linha.size())
			{
				std::stringstream ss;
				ss << "Erro ao tentar iniciar novo circuito com termo invalido. ";
				ss <<  "Linha " << linha_N << ":" << std::endl << Linha;
				throw std::runtime_error(ss.str());
			}
			std::string circuito_nome = removido_vazio_inicio_e_fim(Linha.substr(pos_procura, Linha.npos));
			verbete_circuito.linha_origem = linha_N;
			verbete_circuito.Nome = circuito_nome;
		}
		void inicia_nova_analise(std::string Linha, ind_t linha_N)
		{
			static const std::string termo(u8"Análise");
			auto pos_procura = Linha.find(termo, 0);
			if(pos_procura == termo.npos)
			{
				std::stringstream ss;
				ss << "Erro ao tentar iniciar nova analise com linha invalida. ";
				ss <<  "Linha " << linha_N << ":" << std::endl << Linha;
				throw std::runtime_error(ss.str());
			}
			pos_procura += termo.length();
			if(pos_procura>=Linha.size())
			{
				std::stringstream ss;
				ss << "Erro ao tentar iniciar nova analise com linha invalida. ";
				ss <<  "Linha " << linha_N << ":" << std::endl << Linha;
				throw std::runtime_error(ss.str());
			}
			std::stringstream stream_resto_da_linha(Linha.substr(pos_procura, Linha.npos));
			std::string analise_tipo;
			stream_resto_da_linha >> analise_tipo;
			std::string const etiqueta_quiescente("Quiescente");
			std::string const etiqueta_transiente("Transiente");
			if(analise_tipo == etiqueta_quiescente)
			{
				analises.emplace_back(analise_e_objetivos_t(Verbete_Analise_t::cria_quiescente(linha_N)));
			}else if(analise_tipo == etiqueta_transiente)
			{
				NUMs_t parametros;
				auto extrato=tenta_extrair_numero(stream_resto_da_linha);
				for(; extrato.first; extrato=tenta_extrair_numero(stream_resto_da_linha) )
					{ parametros.emplace_back(extrato.second); }
				if(parametros.size()!=2)
				{
					std::stringstream ss;
					ss << "Erro ao tentar iniciar nova analise com tipo invalido. ";
					ss << "Linha " << linha_N << " :" << std::endl << Linha << std::endl;
					ss << "tipo invalido: " << analise_tipo << std::endl;
					ss << "tipos suportados: " << etiqueta_quiescente << ", " << etiqueta_transiente;
					throw std::runtime_error(ss.str());
				}
				analises.emplace_back(analise_e_objetivos_t(Verbete_Analise_t::cria_transiente(linha_N, parametros)));
			}
			else
			{
				std::stringstream ss;
				ss << "Erro ao tentar iniciar nova analise com tipo invalido. ";
				ss << "Linha " << linha_N << " :" << std::endl << Linha << std::endl;
				ss << "tipo invalido: " << analise_tipo << std::endl;
				ss << "tipos suportados: " << etiqueta_quiescente << ", " << etiqueta_transiente;
				throw std::runtime_error(ss.str());
			}
		}
		void carrega_novo_comando(std::string Linha, ind_t linha_N)
		{
			std::string const etiqueta_cmd_junta(u8"junta nodos"); //juntar nodos não foi implementado na versão atual, e silenciosamente faz nada
			std::string const etiqueta_cmd_usaGNU(u8"gnuplot");
			std::string const etiqueta_cmd_ponto(u8"usa ponto");
			std::string const etiqueta_cmd_virgula(u8"usa vírgula");
			auto pos = Linha.find('.');
			if((pos == Linha.npos) || (pos==Linha.size()-1))
			{
				std::stringstream ss;
				ss << "Erro ao tentar iniciar nono comando com linha invalida. ";
				ss << "Linha " << linha_N << " :" << std::endl << Linha << std::endl;
				throw std::runtime_error(ss.str());
			}
			std::string resto_da_linha = Linha.substr(pos+1, Linha.npos);
			auto pos_inicio_comando = std::distance(resto_da_linha.begin(), primeiro_char_it(resto_da_linha));
			//veridica se comando fornecido é algum dos válidos
			if((resto_da_linha.find(etiqueta_cmd_junta, pos_inicio_comando))== static_cast<std::string::size_type>(pos_inicio_comando))
			{
				auto pos_inicio_nodos = (pos_inicio_comando + etiqueta_cmd_junta.size());
				std::vector<std::string> nodos_a_agrupar;
				std::string nome_nodo;
				resto_da_linha = resto_da_linha.substr(pos_inicio_nodos, resto_da_linha.npos);
				std::stringstream nodos_stream(resto_da_linha);
				while((nodos_stream>>nome_nodo))
				{
					if(!nome_nodo.empty())
						{ nodos_a_agrupar.emplace_back(); }
				}
				if(!nodos_a_agrupar.empty())
					{ nodos_agrupados.emplace_back(nodos_a_agrupar); }
			}else if((Linha.find(etiqueta_cmd_usaGNU, pos_inicio_comando))==static_cast<std::string::size_type>(pos_inicio_comando))
			{
				deve_usar_gnuplot = true;
			} else if((Linha.find(etiqueta_cmd_ponto, pos_inicio_comando))==static_cast<std::string::size_type>(pos_inicio_comando))
			{
				usa_virgula_como_separador_decimal = false;
			} else if((Linha.find(etiqueta_cmd_virgula, pos_inicio_comando))==static_cast<std::string::size_type>(pos_inicio_comando))
			{
				usa_virgula_como_separador_decimal = true;
			} else
			{
				std::stringstream ss;
				ss << "Erro ao tentar iniciar novo comando invalido. ";
				ss << "Linha " << linha_N << " :" << std::endl << Linha << std::endl;
				ss << "Comandos suportados suportados: " << etiqueta_cmd_junta << ", " << etiqueta_cmd_usaGNU;
				throw std::runtime_error(ss.str());
			}
		}
		void carrega_novo_componente(std::string Linha, ind_t linha_N, Estoque::Estoque_t const & estoque)
		{
			std::stringstream linha_stream(Linha);
			std::string tipo;
			std::string modelo;
			std::string nome;
			if(linha_stream >> tipo)
			{
				if( linha_stream >> modelo )
				{
					if( linha_stream >> nome )
					{
						if(estoque.verifica_se_possui_componente(tipo))
						{
							NUMs_t params;
							auto extrato=tenta_extrair_numero(linha_stream);
							for(; extrato.first; extrato=tenta_extrair_numero(linha_stream) )
							{
								params.emplace_back(extrato.second); 
							}
							Verbete_Componente_t componente(linha_N, tipo, modelo, nome, params);
							std::string const nodos_etiqueta = "entre";
							std::string const condi_etiqueta = "CI=";
							NUMs_t CIs;
							std::string resto_da_linha;
							std::getline(linha_stream, resto_da_linha);
							resto_da_linha = removido_vazio_inicio_e_fim(resto_da_linha);
							auto pos_procura = resto_da_linha.find(condi_etiqueta);
							if(pos_procura == 0)
							{
								linha_stream = std::stringstream( resto_da_linha.substr(condi_etiqueta.size(), resto_da_linha.npos) );
								auto extrato_CI = tenta_extrair_numero(linha_stream);
								if(extrato_CI.first)
								{
									componente.condicoes_iniciais.emplace_back(extrato_CI.second);
									std::getline(linha_stream, resto_da_linha);
								}
								else
								{
									std::stringstream ss;
									ss << "Erro ao tentar criar componente. Aparente CI invalida.";
									ss << " Linha " << linha_N << " :" << std::endl << Linha << std::endl;
									throw std::runtime_error(ss.str());
								}
							}
							verbetes_componentes.emplace_back(std::move(componente));
							resto_da_linha = removido_vazio_inicio_e_fim(resto_da_linha);
							pos_procura = resto_da_linha.find(nodos_etiqueta, 0);
							if(pos_procura == 0)
							{
								linha_stream = std::stringstream( resto_da_linha.substr(nodos_etiqueta.size(), resto_da_linha.npos) );
								std::string nodo;
								while(linha_stream >> nodo)
								{
									this->nodos_declarados.insert(nodo);
									verbetes_componentes.back().conexoes.emplace_back(nodo);
								}
								return;
							}
							else
							{
								std::stringstream ss;
								ss << "Erro ao tentar criar componente. Necessarias conexoes.";
								ss << " Linha " << linha_N << " :" << std::endl << Linha << std::endl;
								throw std::runtime_error(ss.str());
							}
						}
					}
				}
			}
			//erro se qualquer operação falhar
			std::stringstream ss;
			ss << "Erro ao tentar criar componente. ";
			ss << "Linha " << linha_N << " :" << std::endl << Linha << std::endl;
			throw std::runtime_error(ss.str());
		}
		std::unique_ptr<Condicionamento::Analise_Base> converte_para_analise()
		{
			if(analises.empty())
			{
				throw std::runtime_error("Tentando criar analise de ArquivoNetList que nao a contem");
			}
			Verbete_Analise_t const & a = analises.back().analise;
			auto const & objetivos = analises.back().objetivos;
			if(a.tipo == Verbete_Analise_t::classe_t::Quiescente)
			{
				auto   an_pt = std::unique_ptr<Condicionamento::Analise_Base>(new Condicionamento::Analise_Quiescente());
				for(auto const &o : objetivos)
					{ an_pt->Adiciona_Objetivo(o.nome, o.propriedade); }
				return an_pt;
			}
			if(a.tipo == Verbete_Analise_t::classe_t::Transiente)
			{
				auto   an_pt = std::unique_ptr<Condicionamento::Analise_Base>(new Condicionamento::Analise_Transiente(a.parametros[0], a.parametros[1]));
				for(auto const &o : objetivos)
					{ an_pt->Adiciona_Objetivo(o.nome, o.propriedade); }
				return an_pt;
			}
			throw std::runtime_error("Tentando criar analise de ArquivoNetList que nao a contem");
		}
		Condicionamento::Circuito_Esquematico converte_para_esquematico()
		{
			Condicionamento::Circuito_Esquematico E;
			for(auto const & c : verbetes_componentes)
			{
				Condicionamento::Componente_Esquematico C(c.tipo, c.modelo, c.nome, c.parametros);
				E.Adiciona(C, c.conexoes);
				for(auto const CI : c.condicoes_iniciais)
				{
					std::cout << "adicionando " << CI << " a componente " << c.nome;
					E.Condicoes_Iniciais.emplace_back( c.nome, CI );
				}
			}
			E.Nome = verbete_circuito.Nome;
			return E;
		}
		void carrega_novo_objetivo(std::string Linha, ind_t linha_N)
		{
			std::string nome;
			std::string propriedade;
			std::stringstream linha_stream(Linha);
			if(linha_stream >> nome)
			{
				if(possui_componente_nomeado(nome))
				{
					if(linha_stream >> propriedade)
					{
						if(!(analises.empty()))
						{
							analises.back().objetivos.emplace_back(linha_N, nome, propriedade);
							return;
						}
					}
				}
			}
			std::stringstream ss;
			ss << "Erro ao tentar criar objetivo ";
			ss << "Linha " << linha_N << " :" << std::endl << Linha << std::endl;
			throw std::runtime_error(ss.str());
		}
		static NUM_T interpreta_possivel_multiplicador(std::string texto)
		{
			//possíveis multiplicadores do SI
			if(texto.find(u8"n")==0)
				{return 1e-9;}
			if((texto.find(u8"u")==0)||(texto.find(u8"μ")==0))
				{return 1e-6;}
			if(texto.find(u8"m")==0)
				{return 1e-3;}
			if(texto.find(u8"k")==0)
				{return 1e+3;}
			if(texto.find(u8"M")==0)
				{return 1e+6;}
			if(texto.find(u8"G")==0)
				{return 1e+9;}
			return NUM_T(1);
		}
		std::pair<bool, NUM_T> tenta_extrair_numero(std::stringstream & stream)
		{
			NUM_T v;
			define_separador_decimal(stream, usa_virgula_como_separador_decimal);
			if(!(stream>>v))
				{
					stream.clear(); //clear apenas resseta o bit de falha de leitura
					return std::pair<bool, NUM_T>{false, NUM_T(0)};
				}
			//possível multiplicador
			if(!(std::iswspace(stream.peek())))
				{
					std::string apendice;
					stream >> apendice;
					v *= interpreta_possivel_multiplicador(apendice);
				}
			return std::pair<bool, NUM_T>{true, v};
		}
	protected:
		template <class charT, charT sep>
		class punct_facet: public std::numpunct<charT> {
			protected:
				charT do_decimal_point() const { return sep; }
		};
		static void define_separador_decimal(std::stringstream & stream, bool usa_virgula)
		{
			if(usa_virgula)
			{
				stream.imbue(std::locale(std::cout.getloc(), new punct_facet<char, ','>));
			}
			else
			{
				stream.imbue(std::locale(std::cout.getloc(), new punct_facet<char, '.'>));
			}
		}
		static typename std::string::iterator primeiro_char_it(std::string & linha)
		{
			auto pos_it = linha.begin();
			while ( pos_it!=linha.end() )
			{
				if(!std::iswspace(*pos_it))
					{ return pos_it;}
				pos_it++;
			}
			return pos_it;
		}
		static char primeiro_char(std::string linha)
		{
			if(primeiro_char_it(linha)!=linha.end())
				{return *primeiro_char_it(linha);}
			else
				{return '\0';}
		}
		static std::string primeira_palavra(std::string linha)
		{
			std::stringstream ss(linha);
			std::string palavra;
			ss >> palavra;
			return palavra;
		}
		static std::string removido_vazio_inicio_e_fim(std::string linha)
		{
			auto i = std::find_if(linha.begin(), linha.end(), [](char a)->bool{return !(std::iswspace(a));});
			auto f = linha.end();
			while(f!=i)
			{
				f--;
				if(!(std::iswspace(*f)))
					{ f++; break; }
			}
			return std::string(i, f);
		}
	};
	inline ArquivoNetList AbrirArquivoNetList(std::string nome_arquivo, Estoque::Estoque_t const & estoque)
	{
		std::ifstream arq(nome_arquivo);
		if(!arq)
		{
			std::stringstream ss;
			ss << "Erro ao abrir " << nome_arquivo << ".";
			throw std::runtime_error(ss.str()); 
		}
		bool declarando_circuito  = false;
		bool declarando_objetivos = false;
		ind_t linha_N = 0;
		ArquivoNetList netlist;
		std::string Linha;
		while (std::getline(arq, Linha))
		{
			linha_N ++;
			Linha = netlist.removido_texto_pos_cerquilha(Linha);
			if(netlist.verifica_linha_vazia(Linha))
				{ continue; }
			if(netlist.verifica_linha_nova_parte(Linha, u8"Circuito"))
			{
				netlist.inicia_novo_circuito(Linha, linha_N);
				declarando_circuito = true;
			}
			else if(netlist.verifica_linha_nova_parte(Linha, u8"Análise"))
			{
				netlist.inicia_nova_analise(Linha, linha_N);
				declarando_circuito = false;
				declarando_objetivos = true;
			}
			else if(netlist.verifica_linha_novo_comando(Linha))
			{
				netlist.carrega_novo_comando(Linha, linha_N);
			}
			else if(declarando_circuito)
			{
				netlist.carrega_novo_componente(Linha, linha_N, estoque);
			}
			else if(declarando_objetivos)
			{
				netlist.carrega_novo_objetivo(Linha, linha_N);
			} else
			{
				std::stringstream ss;
				ss << "Erro ao interpretar " << nome_arquivo << ":\n";
				ss << "  Erro ao interpretar linha " << linha_N << ":\n";
				ss << "  " << Linha << "\n";
				ss << "Verbete indeterminado ou fora de contexto.";
				throw std::runtime_error(ss.str());
			}
		}
		return netlist;
	}
}

#endif //fim header guard SIMULADOR_LEITOR_IVO_INCLUSO