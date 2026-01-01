# Lista de Verificação e Instruções de Release

## Antes de Criar um Release

### 1. Atualizar Números de Versão
- [ ] Atualizar versão no `buildspec.json`
- [ ] Atualizar versão no `CHANGELOG.md` e `CHANGELOG.pt-BR.md`
- [ ] Atualizar versão no comando do script de release

### 2. Testar Tudo
- [ ] Testar em instalação limpa do OBS
- [ ] Testar escaneamento de rede
- [ ] Testar conexão manual
- [ ] Testar atualizações de fontes
- [ ] Testar em Inglês e Português
- [ ] Testar histórico de conexões
- [ ] Testar detecção de mudança de rede

### 3. Compilar Pacote de Release

```powershell
.\build-release.ps1 -Version "1.0.0"
```

### 4. Verificar Conteúdo do Pacote

O ZIP deve conter:
```
obs-holyrics-finder-1.0.0/
??? obs-plugins/
?   ??? 64bit/
?       ??? obs-holyrics-finder.dll
?       ??? obs-holyrics-finder.pdb
??? INSTALL.txt
??? README.md
??? LICENSE
```

## Criando o Release no GitHub

### 1. Criar e Enviar Tag

```bash
git tag -a v1.0.0 -m "Release versão 1.0.0"
git push origin v1.0.0
```

### 2. Criar Release no GitHub

1. Vá para: https://github.com/Pelezi/obs-holyrics-plugin-finder/releases/new
2. Escolha a tag que você acabou de criar (v1.0.0)
3. Defina o título do release: `v1.0.0 - Lançamento Inicial`
4. Use o template abaixo para as notas de release
5. Envie o arquivo ZIP da pasta `release/`
6. Publique o release

### 3. Template de Notas de Release

```markdown
# OBS Holyrics Finder v1.0.0

**Encontre e conecte-se a instâncias do Holyrics na sua rede!**

## ?? Primeiro Lançamento

Este é o lançamento inicial do plugin OBS Holyrics Finder para OBS Studio.

## ? Recursos

- ?? **Scanner de Rede**: Encontre automaticamente instâncias do Holyrics na sua rede
- ?? **Teste Rápido**: Teste conexão com endereços IP específicos
- ?? **Histórico Inteligente**: Lembra e prioriza conexões bem-sucedidas anteriores
- ?? **Atualização em Massa**: Atualize múltiplas fontes de navegador de uma vez
- ?? **Multi-idioma**: Suporte completo para English e Português (Brasil)
- ?? **Copiar para Área de Transferência**: Cópia rápida de IP:Porta
- ? **Seleção Inteligente**: Seleciona apenas fontes que realmente precisam de atualização

## ?? Instalação

1. Baixe `obs-holyrics-finder-1.0.0-windows-x64.zip`
2. Extraia o arquivo ZIP
3. Copie a pasta `obs-plugins` para: `C:\Program Files\obs-studio\`
4. Reinicie o OBS Studio
5. Encontre o plugin em **Ferramentas** ? **Localizador Holyrics**

Para instruções detalhadas, veja [INSTALL.txt](https://github.com/Pelezi/obs-holyrics-plugin-finder#installation) no pacote.

## ?? Requisitos

- Windows 10/11 (64-bit)
- OBS Studio 28.0 ou posterior
- Holyrics rodando na sua rede

## ?? Novidades

- Lançamento inicial com capacidades completas de escaneamento de rede
- Histórico de conexões com priorização inteligente
- Suporte multi-idioma (Inglês + Português)
- Detecção e atualização inteligente de fontes de navegador

## ?? Problemas Conhecidos

Nenhum no momento. Por favor, relate quaisquer problemas na [página de Issues](https://github.com/Pelezi/obs-holyrics-plugin-finder/issues).

## ?? Documentação

- [README Completo](https://github.com/Pelezi/obs-holyrics-plugin-finder/blob/master/README.pt-BR.md)
- [Histórico de Mudanças](https://github.com/Pelezi/obs-holyrics-plugin-finder/blob/master/CHANGELOG.pt-BR.md)

## ?? Agradecimentos

Obrigado a todos que ajudaram a testar e fornecer feedback!

---

**Histórico Completo de Mudanças**: https://github.com/Pelezi/obs-holyrics-plugin-finder/commits/v1.0.0
```

## Pós-Release

### 1. Verificar Release
- [ ] Baixar o ZIP do release
- [ ] Testar instalação em sistema limpo
- [ ] Verificar que todos os recursos funcionam

### 2. Anunciar
- [ ] Atualizar descrição do repositório
- [ ] Compartilhar em comunidades relevantes
- [ ] Atualizar qualquer documentação externa

### 3. Monitorar
- [ ] Observar por issues
- [ ] Responder a perguntas
- [ ] Planejar próxima versão baseado no feedback
