import random

def gerar_input_grande(nome_arquivo, tamanho_dna, qtd_doencas):
    bases = ['A', 'C', 'G', 'T']
    
    # 1. Configurações
    K = 4  # Tamanho da subcadeia
    
    print(f"Gerando DNA com {tamanho_dna} bases...")
    
    # 2. Gerar DNA gigante
    # random.choices é mais rápido para grandes quantidades
    dna = "".join(random.choices(bases, k=tamanho_dna))
    
    print(f"Gerando {qtd_doencas} doenças...")
    
    with open(nome_arquivo, "w") as f:
        # Escreve K
        f.write(f"{K}\n")
        
        # Escreve o DNA
        f.write(f"{dna}\n")
        
        # Escreve quantidade de doenças
        f.write(f"{qtd_doencas}\n")
        
        # Gera e escreve cada doença
        for i in range(qtd_doencas):
            # Gera um código aleatório tipo "DOENCA_001"
            codigo = f"DOENCA_{i+1:04d}"
            
            # Número aleatório de genes (entre 1 e 20 por doença)
            qtd_genes = random.randint(1, 20)
            
            f.write(f"{codigo} {qtd_genes}")
            
            # Gera os genes
            for _ in range(qtd_genes):
                # Tamanho do gene entre 50 e 200 caracteres (como no slide)
                tam_gene = random.randint(50, 200)
                
                # Para garantir que ALGUNS genes sejam encontrados, 
                # vamos clonar pedaços do DNA original aleatoriamente às vezes
                if random.random() < 0.3: # 30% de chance de ser um pedaço real do DNA
                    inicio = random.randint(0, tamanho_dna - tam_gene)
                    gene = dna[inicio : inicio + tam_gene]
                else:
                    # Gene totalmente aleatório
                    gene = "".join(random.choices(bases, k=tam_gene))
                
                f.write(f" {gene}")
            
            f.write("\n")

    print(f"Arquivo '{nome_arquivo}' gerado com sucesso!")

# --- CONFIGURE AQUI O TAMANHO ---
# Para testar "Input Grande", tente:
# DNA com 1 milhão de caracteres
# 1000 doenças
gerar_input_grande("input_grande.txt", tamanho_dna=1000000, qtd_doencas=1000)