# Plugin OBS Holyrics Finder

**Encontre e conecte-se a instâncias do Holyrics na sua rede diretamente do OBS Studio!**

[![GitHub release](https://img.shields.io/github/v/release/Pelezi/obs-holyrics-plugin-finder)](https://github.com/Pelezi/obs-holyrics-plugin-finder/releases)
[![License](https://img.shields.io/github/license/Pelezi/obs-holyrics-plugin-finder)](LICENSE)

> [Read in English](README.md)

Este plugin para OBS Studio ajuda você a encontrar e configurar rapidamente fontes de navegador apontando para instâncias da [visualização de palco do Holyrics](https://holyrics.com.br/) na sua rede local.

## Recursos

- **Scanner de Rede**: Escaneie automaticamente sua rede para encontrar instâncias do Holyrics
- **Teste Rápido**: Teste a conexão com um endereço IP específico
- **Histórico Inteligente**: Lembra das conexões bem-sucedidas e as testa primeiro
- **Atualização em Massa**: Atualize múltiplas fontes de navegador com um clique
- **Multi-idioma**: Suporta Inglês e Português (Brasil)
- **Copiar IP:Porta**: Funcionalidade de cópia para área de transferência
- **Seleção Inteligente**: Seleciona automaticamente apenas as fontes que precisam ser atualizadas

## Instalação

### Requisitos

- **OBS Studio** 32.0 ou posterior
- **Windows** 10/11 (64-bit)
- **Holyrics** rodando na sua rede

### Passos

1. Baixe o ZIP da versão mais recente em [Releases](https://github.com/Pelezi/obs-holyrics-plugin-finder/releases)
2. Extraia o arquivo ZIP
3. Copie a pasta `obs-plugins` para o diretório de instalação do OBS
   - Padrão: `C:\Program Files\obs-studio\`
4. Reinicie o OBS Studio
5. Encontre o plugin em **Ferramentas** > **Localizador Holyrics**

## Uso
### Encontrando o Holyrics na Sua Rede

1. Abra o OBS Studio
2. Vá em **Ferramentas** > **Localizador Holyrics**
3. Escolha uma destas opções:
   - **Testar Conexão**: Se você souber o endereço IP
   - **Escanear Rede**: Para encontrar automaticamente o Holyrics

### Atualizando Fontes de Navegador

1. Após uma conexão bem-sucedida, as fontes são listadas automaticamente
2. Fontes que precisam ser atualizadas são selecionadas automaticamente
3. Fontes já apontando para o IP:Porta correto ficam desmarcadas
4. Clique em **Atualizar Fontes Selecionadas** para aplicar as mudanças

### Dicas

- O plugin lembra das conexões bem-sucedidas
- No próximo escaneamento, testa os IPs anteriores primeiro (mais rápido!)
- Se você mudar de rede, ele se adapta automaticamente
- Use "Copiar IP:Porta" para compartilhar detalhes da conexão rapidamente

## Idiomas Suportados

O plugin automaticamente corresponde à configuração de idioma do OBS:
- **English**
- **Português (Brasil)**

Para mudar o idioma:
1. Vá em **Configurações** do OBS → **Geral** → **Idioma**
2. Reinicie o OBS

## Compilando do Código Fonte
### Pré-requisitos

- CMake 3.16 ou posterior
- Visual Studio 2026 com ferramentas C++
- Qt 6
- Dependências de compilação do OBS Studio 32.x

### Passos de Compilação

```powershell
# Clone o repositório
git clone https://github.com/Pelezi/obs-holyrics-plugin-finder.git
cd obs-holyrics-plugin-finder

# Configure
cmake --preset windows-x64

# Compile Release
cmake --build build_x64_ninja --config Release

# Ou use o script de implantação para testes locais
.\build-and-deploy.ps1
```

### Criando um Pacote de Release

```powershell
.\build-release.ps1 -Version "1.0.0"
```

Isso cria um arquivo ZIP pronto para distribuição na pasta `release`.

## Como Funciona

1. **Escaneamento de Rede**: Testa conexões com todos os IPs na sua sub-rede (XXX.XXX.XXX.1-254)
2. **Detecção Inteligente**: Identifica o Holyrics verificando respostas HTTP
3. **Prioridade do Histórico**: Testa IPs bem-sucedidos anteriormente primeiro
4. **Atualização Automática**: Seleciona apenas fontes que apontam para IPs diferentes

## Solução de Problemas

### Plugin não aparece no menu Ferramentas
- Verifique se você copiou os arquivos para o diretório correto do OBS
- Confira a versão do OBS (deve ser 32.0+)
- Veja os logs do OBS: **Ajuda** > **Arquivos de Log** > **Ver Log Atual**

### Não consegue encontrar a instância do Holyrics

- Certifique-se de que o Holyrics está rodando e acessível na sua rede
- Verifique as configurações do firewall
- Tente testar com o endereço IP específico ao invés de escanear
- A porta padrão do Holyrics é 80

### Fontes não atualizam

- Certifique-se de que as fontes são fontes de navegador com URLs IP:Porta
- Verifique se as fontes estão na coleção de cenas atual
- Tente clicar em "Atualizar Lista"

## Contribuindo

Contribuições são bem-vindas! Sinta-se à vontade para enviar um Pull Request.

## Licença
Este projeto está licenciado sob a Licença Pública Geral GNU v2.0 - veja o arquivo [LICENSE](LICENSE) para detalhes.

## Agradecimentos

- [OBS Studio](https://obsproject.com/) - Software incrível de streaming
- [Holyrics](https://holyrics.com.br/) - Software de apresentação para igrejas
- Todos os contribuidores e usuários deste plugin

## Suporte

- **Relatar Bugs**: [GitHub Issues](https://github.com/Pelezi/obs-holyrics-plugin-finder/issues)
- **Solicitar Recursos**: [GitHub Issues](https://github.com/Pelezi/obs-holyrics-plugin-finder/issues)

---

**Feito com ❤️ para a comunidade de tecnologia cristã**