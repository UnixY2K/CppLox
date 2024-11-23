#pragma once

#include <string>
#include <variant>

namespace lox {

// helper type for the visitor
template <class... Ts> struct overloads : Ts... {
	using Ts::operator()...;
};

class Object {};
class Chunk;

class ChunkHolder {
	Chunk *chunk;

  public:
	ChunkHolder();
	ChunkHolder(const Chunk &chunk);
	ChunkHolder(Chunk *chunk);
	ChunkHolder(ChunkHolder &&) noexcept;
	ChunkHolder(const ChunkHolder &);
	ChunkHolder &operator=(const ChunkHolder &);
	ChunkHolder &operator=(ChunkHolder &&) noexcept;
	ChunkHolder &operator=(const Chunk &);
	ChunkHolder &operator=(Chunk &&) noexcept;

	Chunk& get() const;

	Chunk &operator*() const;
	Chunk *operator->() const;

	~ChunkHolder();
};

struct ObjFunction {
	Object obj;
	ChunkHolder chunk;
	size_t arity;
	std::string name;
};

using Obj = std::variant<std::string, ObjFunction>;

std::string objToString(const Obj &obj);

} // namespace lox
