#!/bin/bash

# Создаём папки для выходных файлов, если они ещё не существуют
mkdir -p ../frontend/proto
mkdir -p cpp

# Ищем все .proto файлы в текущей директории
for proto_file in *.proto; do
  # Генерируем JavaScript файлы с помощью protobufjs
  # protoc --js_out=import_style=commonjs,binary:js "$proto_file"
  # npx pbjs -t static-module -w es6 -o "../frontend/proto/${proto_file%.proto}_pb.js" "$proto_file"
  cp "$proto_file" "../frontend/proto/$proto_file"

  # Генерируем C++ файлы с помощью protoc
  protoc --cpp_out=cpp "$proto_file"
done

echo "Генерация завершена. JavaScript файлы находятся в папке '../frontend/proto', C++ файлы — в папке 'cpp'."