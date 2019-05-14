g++ -c main.cpp
FILES=()
OBJS=()
for f in "${FILES[@]}"; do
  g++ -c "$f.cpp"
  OBJS+=("$f.o")
done
g++ -W -Wall -g main.o "${OBJS[@]}" -o ca -lSDL2 -lSDL2_image -I/usr/local/include -L/usr/local/lib -llua
