# ImageSession

`ImageSession` é o contrato compartilhado de imagens do PurrView. Ela é um `QAbstractListModel`
pertencente ao `ApplicationContext` do Shell e deve ser manipulada na thread da interface.

Cada `ImageEntry` mantém somente estado leve: `QUuid`, caminho absoluto, nome, dimensões, rotação
lógica, seleção e validade. Imagens em resolução total não são mantidas na sessão.

## Consistência

- Sessão vazia possui `currentIndex == -1`.
- A primeira imagem adicionada torna-se atual.
- Remover uma imagem antes da atual corrige o índice sem mudar sua identidade.
- Remover a imagem atual escolhe a próxima; se ela era a última, escolhe a anterior.
- Seleção e imagem atual são estados independentes.
- Rotação é lógica, limitada a `0`, `90`, `180` e `270`, sem modificar o arquivo.

## Paths e validação

Internamente são usados caminhos absolutos em `QString`. A role `source` converte o caminho para
`QUrl` ao expô-lo ao QML. O conjunto público é PNG, JPEG, WebP, BMP, GIF e TIFF, limitado aos
decodificadores que `QImageReader` realmente disponibiliza em tempo de execução. Os aliases
`jpg/jpeg` e `tif/tiff` são tratados em conjunto. A validação lê somente as informações necessárias
de formato e dimensões.

O `FolderImageModel` não é parte da sessão: ele representa todo o catálogo da pasta. Arquivos com
extensão reconhecida, mas conteúdo corrompido, permanecem navegáveis como referências inválidas para
que o Viewer mostre o fallback e permita seguir à imagem seguinte.

Duplicatas são permitidas intencionalmente, mas cada entrada recebe um `QUuid` independente.
