# Como publicar a Nexora no GitHub

## Criar o repositório

1. No GitHub, crie um repositório público chamado `nexora` sem adicionar README, licença ou `.gitignore`.
2. Abra um terminal na pasta do código-fonte.
3. Remova apenas o vínculo com o repositório antigo, se existir, e configure o seu endereço:

```powershell
git remote remove origin
git remote add origin https://github.com/SEU_USUARIO/nexora.git
git add .
git commit -m "Nexora 1.0.0"
git branch -M main
git push -u origin main
```

O histórico original pode ser mantido — isso dá transparência à autoria. Se preferir um histórico novo, copie os arquivos do ZIP de source para um diretório vazio e execute `git init` antes dos comandos acima.

## Gerar os programas automaticamente

1. Abra a aba **Actions** do repositório e habilite os workflows, caso o GitHub solicite.
2. Confirme que **Build Android APK** e **Build Windows** terminam em verde.
3. Crie a primeira versão:

```powershell
git tag v1.0.0
git push origin v1.0.0
```

O workflow **Create release** compila, empacota e publica os arquivos na página Releases. Confira os nomes e hashes antes de divulgar.

## Atualizações futuras

Altere `versionName` e `versionCode` em `android/app/build.gradle.kts`, faça commit e crie uma nova tag. Use versionamento `MAJOR.MINOR.PATCH`, por exemplo `v1.0.1` para correção, `v1.1.0` para recurso novo e `v2.0.0` para mudança incompatível.

## Licença

Não remova a linha de copyright do autor original. A licença MIT permite modificar e redistribuir, mas exige que o aviso original e o texto da licença acompanhem cópias substanciais. A sua linha, `Copyright (c) 2026 Yves Godoy (Nexora modifications)`, já foi acrescentada.
