#include <Condicionamento/Circuito_Esquematico.hpp>
#include <Condicionamento/Analise_Quiescente.hpp>

//esquemático usado para circuito de teste
Condicionamento::Circuito_Esquematico Esquema;

auto const estoque = Estoque::Estoque_t::Estoque_Pre_Compilado();

void teste_controle_base()
{
	//monta esquemático
		Condicionamento::Componente_Esquematico V1("FonteDDPfixa", "ideal", "V1", 8);
		Condicionamento::Componente_Esquematico RA("Resistor", "ideal", "RA", 2);
		Condicionamento::Componente_Esquematico RB("Resistor", "ideal", "RB", 4);
		Condicionamento::Componente_Esquematico RC("Resistor", "ideal", "RC", 5);
		Esquema.Adiciona(V1, "n1", "0");
		Esquema.Adiciona(RA, "n1", "nd");
		Esquema.Adiciona(RB, "nd", "0");
		Esquema.Adiciona(RC, "n1", "0");
		
	//monta análise e objetivos desta	
		Condicionamento::Analise_Quiescente analise_Q1;
		analise_Q1.Adiciona_Objetivo("V1", "i");
		analise_Q1.Adiciona_Objetivo("RA", "i");
		analise_Q1.Adiciona_Objetivo("nd");
		
	//resolve circuito
		auto resultado = analise_Q1.resolve(Esquema, estoque);
			
	//imprime resultados
		std::cout << resultado << std::endl;
}

void teste_controle_I_de_I()
{
	//adiciona componentes ao esquemático
		Condicionamento::Componente_Esquematico Ic1("FonteIControladaI", "ideal", "Ic1", 3);
		Condicionamento::Componente_Esquematico Rc1("Resistor", "ideal", "Rc1", 1);
		Condicionamento::Componente_Esquematico Rc2("Resistor", "ideal", "Rc2", 2);
		Esquema.Adiciona(Ic1, "0", "n2", "RA");
		Esquema.Adiciona(Rc1, "n2", "0");
		Esquema.Adiciona(Rc2, "n2", "0");
		
	//monta análise e objetivos desta	
		Condicionamento::Analise_Quiescente analise_Q1;
		analise_Q1.Adiciona_Objetivo("Ic1", "i");
		analise_Q1.Adiciona_Objetivo("n2");
		
	//resolve circuito
		auto resultado = analise_Q1.resolve(Esquema, estoque);
			
	//imprime resultados
		std::cout << resultado << std::endl;
}

void teste_controle_I_de_V()
{
	//adiciona componentes ao esquemático
		Condicionamento::Componente_Esquematico Ic2("FonteIControladaV", "ideal", "Ic2", 3);
		Condicionamento::Componente_Esquematico Rf1("Resistor", "ideal", "Rf1", 1);
		Condicionamento::Componente_Esquematico Rf2("Resistor", "ideal", "Rf2", 2);
		Esquema.Adiciona(Ic2, "0", "n3", "nd", "n2");
		Esquema.Adiciona(Rf1, "n3", "0");
		Esquema.Adiciona(Rf2, "n3", "0");
		
	//monta análise e objetivos desta	
		Condicionamento::Analise_Quiescente analise_Q1;
		analise_Q1.Adiciona_Objetivo("Ic2", "i");
		analise_Q1.Adiciona_Objetivo("n3");
		
	//resolve circuito
		auto resultado = analise_Q1.resolve(Esquema, estoque);
			
	//imprime resultados
		std::cout << resultado << std::endl;
}

int main()
{
	teste_controle_base();
	teste_controle_I_de_I();
	teste_controle_I_de_V();
};