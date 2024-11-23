#include <cpplox/chunk.hpp>
#include <cpplox/obj.hpp>

#include <format>

namespace lox {

ChunkHolder::ChunkHolder() : chunk(new Chunk()) {}

ChunkHolder::ChunkHolder(Chunk *chunk) : chunk(chunk) {}

ChunkHolder::ChunkHolder(const Chunk &chunk) : chunk(new Chunk(chunk)) {}

ChunkHolder::ChunkHolder(ChunkHolder &&other) noexcept : chunk(other.chunk) {
	other.chunk = nullptr;
}

ChunkHolder::ChunkHolder(const ChunkHolder &other)
    : chunk(new Chunk(*other.chunk)) {}

ChunkHolder &ChunkHolder::operator=(const ChunkHolder &other) {
	if (this != &other) {
		// before copying the data, deallocate the current data
		delete chunk;
		chunk = new Chunk(*other.chunk);
	}
	return *this;
}

ChunkHolder &ChunkHolder::operator=(ChunkHolder &&other) noexcept {
	if (this != &other) {
		// before copying the data, deallocate the current data
		delete chunk;
		chunk = other.chunk;
		other.chunk = nullptr;
	}
	return *this;
}

ChunkHolder &ChunkHolder::operator=(const Chunk &other) {
	*chunk = other;
	return *this;
}
ChunkHolder &ChunkHolder::operator=(Chunk &&other) noexcept {
	*chunk = std::move(other);
	return *this;
}

Chunk &ChunkHolder::get() const { return *chunk; }

Chunk &ChunkHolder::operator*() const { return *chunk; }

Chunk *ChunkHolder::operator->() const { return chunk; }

ChunkHolder::~ChunkHolder() { delete chunk; }

std::string objToString(const Obj &obj) {
	std::string result;
	std::visit(
	    overloads{[&result](const std::string &value) { result = value; },
	              [&result](const ObjFunction &value) {
		              if (value.name.empty()) {
			              result = "<script>";
		              } else {
			              result = std::format("<fn {}>", value.name);
		              }
	              }},
	    obj);
	return result;
}
} // namespace lox
