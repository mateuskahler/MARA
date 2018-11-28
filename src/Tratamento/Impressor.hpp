

#ifndef SIMULADOR_TRATAMENTO_IMPRESSOR_INCLUIDO
#define SIMULADOR_TRATAMENTO_IMPRESSOR_INCLUIDO sim

#include <Auxiliar/Dogmas.hpp>
#include <Tratamento/Extrator.hpp>
#include <fstream>
#include <cstdlib>

namespace Tratamento
{
	//estas constantes guardam o caminho para execução do GNUplot conforme organizado no repositório
	constexpr char GNU_PLOT_COMANDO_MESMA_PASTA[]   = ".\\gnuplot\\bin\\gnuplot";
	constexpr char GNU_PLOT_COMANDO_RELATIVO_REPO[] = "..\\bin\\gnuplot\\bin\\gnuplot";
	struct Configura_Grafico_t
	{
		ind2_t tamanho_pixels;
		std::string titulo;
		std::string legenda_eixo_ordenadas;
		Configura_Grafico_t()
		{
			//configuração padrão deliberada
			tamanho_pixels = ind2_t{1024,600};
			titulo = std::string(u8"(sem título)");
			legenda_eixo_ordenadas = std::string(u8"");
		}
	};
	
	struct Curva_t
	{
		std::string nome;
		NUMs_t pontos;
	};
	
	struct Relacao_t
	{
		Curva_t abscissa;
		std::vector<Curva_t> ordenadas;
	};
	
	inline bool gera_arquivo_dados(
		std::string nome_arquivo, 
		std::string titulo,
		Relacao_t const & dados )
	{
		std::ofstream arq(nome_arquivo, std::ios::trunc);
		if(!arq)
			{return false;}
		// lambda auxiliar para formatar números com 6 dígitos após a vírgula
			auto proc_v = [&](NUM_T x) -> void
				{
					arq << u8" " << std::setprecision(6) << std::setw(17) << std::scientific << x << u8" ";
				};
		//imprime título dos dados e de cada curva
			arq << u8"# arquivo de dados para \'" << titulo << u8"\'" << std::endl;
			arq << u8"# ";
			arq << std::setw(16) << dados.abscissa.nome;
			for(auto const & curva : dados.ordenadas )
				{ arq << u8"," << std::setw(17) << curva.nome << u8" "; }
			ind_t quantidade_pontos = dados.abscissa.pontos.size();
		//coleta quantidade de pontos
			for(auto const & curva : dados.ordenadas )
				{ quantidade_pontos = std::min(static_cast<ind_t>(curva.pontos.size()), quantidade_pontos); }
		//imprime cada linha
			for(ind_t k=0; k<quantidade_pontos; k++)
				{
					arq << std::endl;
					proc_v(dados.abscissa.pontos[k]);
					for(auto const & curva : dados.ordenadas )
						{ proc_v(curva.pontos[k]); }
				}
			bool arq_ok = bool(arq);
			return arq_ok;
	}
	
	inline bool gera_arquivo_script_gnuplot(
		std::string nome_arquivo_script, 
		std::string nome_arquivo_dados,
		Configura_Grafico_t const & configura_grafico,
		Relacao_t const & dados,
		bool salvar_como_arquivo_png )
	{
		std::ofstream arq(nome_arquivo_script, std::ios::trunc);
		if(!arq)
			{return false;}
		arq << u8"# arquivo de script gnuplot para \'" << configura_grafico.titulo << u8"\'" << std::endl;
		arq << u8"# gerado automaticamente para plotar \'" << nome_arquivo_dados << u8"\'" << std::endl;
		arq << std::endl;
		//configura saída
			arq << u8"# configura saída" << std::endl;
			if(salvar_como_arquivo_png)
				{ arq << u8"set terminal pngcairo "; }
			else
				{	arq << u8"set terminal wxt "; }
			arq << u8"size " << configura_grafico.tamanho_pixels[0] << u8"," << configura_grafico.tamanho_pixels[1] << " ";
			arq << u8"enhanced font \'Verdana,10\'" << std::endl;
			std::string arq_imagem_nome(nome_arquivo_script + u8"_imagem.png");
			if(salvar_como_arquivo_png)
				{ arq << u8"set output \"" << arq_imagem_nome << u8"\""; }
			arq << std::endl;
		//define título do gráfico
			arq << u8"set title \"" << configura_grafico.titulo << u8"\"" << std::endl;
			arq << std::endl;
		//configura linhas
			arq << u8"# Expessura das bordas" << std::endl;
			arq << u8"set border linewidth 1.5" << std::endl;
			arq << std::endl;
			arq << u8"# Estilo das linhas" << std::endl;
			arq << u8"set style line 1 linecolor rgb \'#0060ad\' linetype 1 linewidth 2" << std::endl;
			//set style line 2 linecolor rgb '#dd181f' linetype 1 linewidth 2
			arq << std::endl;
		//posição da Legenda
			arq << u8"# Legenda" << std::endl;
			arq << u8"set key outside bottom center" << std::endl;
			arq << std::endl;
		//nome dos Eixos
			arq << u8"# Nome dos eixos" << std::endl;
			arq << u8"set xlabel \"" << dados.abscissa.nome << "\"";
			arq << std::endl;
			arq << u8"set ylabel \"" << configura_grafico.legenda_eixo_ordenadas << "\"";
			arq << std::endl;
		//limite dos da Eixos
			arq << u8"# Limite dos eixos" << std::endl;
			arq<< u8"set yrange [*:*]" << u8"\n";
			arq<< u8"set xrange [*:*]" << u8"\n";
			arq<< u8"set autoscale" << u8"\n";
		//comando para gerar gráfico de cada curva
			arq << u8"# Evoca desenho do gráfico" << std::endl;
			ind_t const curvas_N = dados.ordenadas.size();
			for(ind_t k = 0; k<curvas_N; k++)
			{
				Curva_t const & curva = dados.ordenadas[k];
				if(k==0)
					{ arq << u8"plot \"" << nome_arquivo_dados << u8"\""; }
				else
					{ arq << u8" \"" << nome_arquivo_dados << u8"\""; }
				arq << u8" using 1:" << k+2;
				arq << u8" title \"" << curva.nome << u8"\"";
				arq << u8" with lines linestyle 1";
				if(k!=(curvas_N-1))
					{ arq << u8", \\";}
				arq << std::endl;
			}
		if(!salvar_como_arquivo_png)
		{
			arq << std::endl;
			arq << u8"# Quando mostrando gráfico no terminal, espera usuário apertar [Enter]" << std::endl;
			arq << u8" pause -1 \"Aperte [Enter] para continuar\"";
		}
		bool arq_ok = bool(arq);
		return arq_ok;
	}
	
	inline void evoca_gnuplot(std::string gnu_comando, std::string nome_arquivo)
	{
		std::string comando(gnu_comando);
		comando += " ";
		comando += "\"";
		comando += nome_arquivo;
		comando += "\"";
		std::cout << "Executando comando: \n\t" << comando << std::endl;
		std::system(comando.c_str());
	}
	
	inline bool salva_resultado_quiescente_em_arquivo(
		std::string nome_arquivo,
		std::string titulo,
		Resultados_t resultados
		)
	{
		//para análise quiescente, a variável independente é ignorada
		std::ofstream arq(nome_arquivo);
		arq << u8"# arquivo de dados para \'" << titulo << u8"\'" << std::endl;
		if(!arq)
			{ return false; }
		arq << resultados;
		return true;
	}
	
	inline bool salva_resultado_transiente_em_arquivo(
		std::string nome_arquivo,
		std::string titulo_grafico,
		Resultados_t resultados,
		bool gerar_script_GNUplot,
		bool usar_gnuplot_para_salvar_como_png,
		ind_t largura_pixels,
		ind_t altura_pixels,
		bool executar_resultado_GNUplot,
		std::string comando_GNUplot )
	{
		std::string nome_arquivo_dados = nome_arquivo + u8"_dados";
		Tratamento::Relacao_t dados;
		dados.abscissa.nome   = u8"tempo [segundos]";
		dados.abscissa.pontos = resultados.variavel_independente;
		if(dados.abscissa.pontos.empty())
		{
			return false;
		}
		for(auto curva_resultado : resultados.resultados)
		{
			Curva_t curva;
			curva.nome   = curva_resultado.origem.nome + u8" " + curva_resultado.origem.propriedade;
			curva.pontos = curva_resultado.valor;
			dados.ordenadas.emplace_back(curva);
		}
		bool arquivo_ok = gera_arquivo_dados(nome_arquivo_dados, titulo_grafico, dados);
		if(gerar_script_GNUplot && arquivo_ok)
		{
			Configura_Grafico_t graf;
			graf.titulo = titulo_grafico;
			graf.legenda_eixo_ordenadas = u8"resultado simulado";
			graf.tamanho_pixels = ind2_t{largura_pixels, altura_pixels};
			arquivo_ok = gera_arquivo_script_gnuplot(nome_arquivo, nome_arquivo_dados, graf, dados, usar_gnuplot_para_salvar_como_png);
			if(executar_resultado_GNUplot && arquivo_ok)
				{ evoca_gnuplot(comando_GNUplot, nome_arquivo); }
		}
		return arquivo_ok;
	}
} //fim namespace Tratamento

#endif // fim header guard SIMULADOR_TRATAMENTO_IMPRESSOR_INCLUIDO