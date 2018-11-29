#include <Condicionamento/Analise_Quiescente.hpp>

//esquemático usado para circuito de teste
Condicionamento::Circuito_Esquematico Esquema;

auto const estoque = Estoque::Estoque_t::Estoque_Pre_Compilado();


void teste_controle_base()
{
	//monta esquemático
		Condicionamento::Componente_Esquematico I1("FonteCorrenteFixa", "ideal", "I1", 3);
		Condicionamento::Componente_Esquematico RA("Resistor", "ideal", "RA", 6);
		Condicionamento::Componente_Esquematico RB("Resistor", "ideal", "RB", 4);
		Condicionamento::Componente_Esquematico RC("Resistor", "ideal", "RC", 2);
		Esquema.Adiciona(I1, "0", "n1");
		Esquema.Adiciona(RA, "n1", "nd");
		Esquema.Adiciona(RB, "nd", "0");
		Esquema.Adiciona(RC, "nd", "0");
		
	//monta análise e objetivos desta
		Condicionamento::Analise_Quiescente analise_Q1;
		analise_Q1.Adiciona_Objetivo("I1", "u");
		analise_Q1.Adiciona_Objetivo("nd");
		
	//resolve circuito
		auto resultado = analise_Q1.resolve(Esquema, estoque);
			
	//imprime resultados
		std::cout << resultado << std::endl;
}

void teste_controle_V_de_V()
{
	//adiciona componentes ao esquemático
		Condicionamento::Componente_Esquematico Vc1("FonteVControladaV", "ideal", "Vc1", 2);
		Condicionamento::Componente_Esquematico R1("Resistor", "ideal", "R1", 12);
		Condicionamento::Componente_Esquematico R2("Resistor", "ideal", "R2", 18);
		Condicionamento::Componente_Esquematico R3("Resistor", "ideal", "R3", 9);
		Esquema.Adiciona(Vc1, "n2", "0", "nd", "ne");
		Esquema.Adiciona(R1, "n2", "ne");
		Esquema.Adiciona(R2, "ne", "0");
		Esquema.Adiciona(R3, "ne", "0");
		
	//monta análise e objetivos desta
		Condicionamento::Analise_Quiescente analise_Q1;
		analise_Q1.Adiciona_Objetivo("Vc1", "u");
		analise_Q1.Adiciona_Objetivo("R3", "i");
		
	//resolve circuito
		auto resultado = analise_Q1.resolve(Esquema, estoque);
			
	//imprime resultados
		std::cout << resultado << std::endl;
}

void teste_controle_V_de_I()
{
	//adiciona componentes ao esquemático
		Condicionamento::Componente_Esquematico Vc2("FonteVControladaI", "ideal", "Vc2", 45);
		Condicionamento::Componente_Esquematico Z1("Resistor", "ideal", "Z1", 2);
		Condicionamento::Componente_Esquematico Z2("Resistor", "ideal", "Z2", 4);
		Condicionamento::Componente_Esquematico Z3("Resistor", "ideal", "Z3", 4);
		Esquema.Adiciona(Vc2, "n3", "0", "R3");
		Esquema.Adiciona(Z1, "n3", "nf");
		Esquema.Adiciona(Z2, "nf", "0");
		Esquema.Adiciona(Z3, "nf", "0");
		
	//monta análise e objetivos desta
		Condicionamento::Analise_Quiescente analise_Q1;
		analise_Q1.Adiciona_Objetivo("Vc2", "i");
		analise_Q1.Adiciona_Objetivo("n3");
		
	//resolve circuito
		auto resultado = analise_Q1.resolve(Esquema, estoque);
			
	//imprime resultados
		std::cout << resultado << std::endl;
}

int main()
{
	teste_controle_base();
	teste_controle_V_de_V();
	teste_controle_V_de_I();
};