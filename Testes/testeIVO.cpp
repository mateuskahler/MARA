#include <LeitorNetlist_IVO/Ivo.hpp>
#include <Tratamento/Impressor.hpp>

int main(int argc, char **argv)
{
	auto estoque = Estoque::Estoque_t::Estoque_Pre_Compilado();
	auto netlist = IVONetList::AbrirArquivoNetList("IvoREF.txt", estoque);
	auto esquema = netlist.converte_para_esquematico();
	auto analise = netlist.converte_para_analise();
	std::cout << esquema << std::endl;
	auto resultado = analise->resolve(esquema, estoque);
	std::cout << resultado << std::endl;
	Tratamento::salva_resultado_transiente_em_arquivo("nome_arquivo", esquema.Nome, resultado, true, true, 1000, 600, true, Tratamento::GNU_PLOT_COMANDO_RELATIVO_REPO);
}