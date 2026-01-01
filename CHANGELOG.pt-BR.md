# Histórico de Mudanças

> [Read in English](CHANGELOG.md)

Todas as mudanças notáveis no Plugin OBS Holyrics Finder serão documentadas neste arquivo.

O formato é baseado em [Keep a Changelog](https://keepachangelog.com/pt-BR/1.0.0/),
e este projeto adere ao [Versionamento Semântico](https://semver.org/lang/pt-BR/spec/v2.0.0.html).

## [1.0.0] - 2024-01-XX

### Adicionado
- Lançamento inicial do Plugin OBS Holyrics Finder
- Scanner de rede para encontrar automaticamente instâncias do Holyrics
- Teste de conexão manual para endereços IP conhecidos
- Histórico de conexões com priorização inteligente
- Atualização em massa de URLs de fontes de navegador
- Suporte a múltiplos idiomas (Inglês e Português Brasil)
- Seleção automática de fontes que precisam ser atualizadas
- Funcionalidade de copiar IP:Porta para área de transferência
- Armazenamento persistente de configurações
- Detecção de mudança de rede
### Recursos
- Escaneia faixa de rede (XXX.XXX.XXX.1-254) para instâncias do Holyrics
- Testa conexões históricas primeiro para escaneamento mais rápido
- Detecta automaticamente a configuração de idioma do OBS
- Seleção inteligente de fontes (seleciona apenas fontes que precisam atualização)
- Timeout de 2 segundos por teste de conexão
- Suporta configuração de porta personalizada
- Feedback visual com mensagens de status e barra de progresso

### Plataformas Suportadas
- Windows 10/11 (64-bit)
- OBS Studio 32.0 ou posterior

### Limitações Conhecidas
- Apenas Windows

## [Não Lançado]

### Recursos Planejados
- Auto-criação de fontes de navegador se não existirem