#include "json_builder.h"

using namespace std::literals;

namespace json {
	DictItemContext::DictItemContext(Builder& builder) : builder_(builder) {}

	KeyItemContext DictItemContext::Key(std::string key) {
		return builder_.Key(std::move(key));
	}

	Builder& DictItemContext::EndDict() {
		return builder_.EndDict();
	}

	ArrayItemContext::ArrayItemContext(Builder& builder) : builder_(builder) {}

	ArrayItemContext ArrayItemContext::Value(Node::Value value) {
		return builder_.Value(std::move(value));
	}

	DictItemContext ArrayItemContext::StartDict() {
		return builder_.StartDict();
	}

	ArrayItemContext ArrayItemContext::StartArray() {
		return builder_.StartArray();
	}

	Builder& ArrayItemContext::EndArray() {
		return builder_.EndArray();
	}

	KeyItemContext::KeyItemContext(Builder& builder) : builder_(builder) {}

	DictItemContext KeyItemContext::Value(Node::Value value) {
		return builder_.Value(std::move(value));
	}

	DictItemContext KeyItemContext::StartDict() {
		return builder_.StartDict();
	}

	ArrayItemContext KeyItemContext::StartArray() {
		return builder_.StartArray();
	}

	KeyItemContext Builder::Key(std::string key) {
		if (current_method_ == MethodTypes::KEY) {
			throw std::logic_error("current_method_ == MethodTypes::KEY"s);
		}

		if (nodes_stack_.empty()) {
			throw std::logic_error("nodes_stack_.empty()"s);
		}

		if (!nodes_stack_.back()->IsDict()) {
			throw std::logic_error("!nodes_stack_.back()->IsDict()"s);
		}

		key_.push_back(std::move(key));
		current_method_ = MethodTypes::KEY;

		return *this;
	}

	Builder& Builder::Value(Node::Value value) {
		if (current_method_ == MethodTypes::BEFORE_START) {
			root_ = GetNode(std::move(value));
		}
		else if (!nodes_stack_.empty() && nodes_stack_.back()->IsDict() && current_method_ == MethodTypes::KEY) {
			std::map<std::string, Node>& m = const_cast<Dict&>(nodes_stack_.back()->AsDict());
			m.insert({ key_.back(), GetNode(std::move(value)) });
		}
		else if (!nodes_stack_.empty() && nodes_stack_.back()->IsArray()) {
			std::vector<Node>& m = const_cast<Array&>(nodes_stack_.back()->AsArray());
			m.emplace_back(std::move(GetNode(std::move(value))));
		}
		else {
			throw std::logic_error("Value can not will be here"s);
		}
		current_method_ = MethodTypes::VALUE;
		return *this;
	}

	DictItemContext Builder::StartDict() {
		
		CheckStart("StartDict"s);

		nodes_.emplace_back(std::move(Dict{}));
		nodes_stack_.emplace_back(&nodes_.back());
		current_method_ = MethodTypes::START_DICT;

		return { *this };
	}

	ArrayItemContext Builder::StartArray() {

		CheckStart("StartArray"s);

		nodes_.emplace_back(std::move(Array{ }));
		nodes_stack_.emplace_back(&nodes_.back());
		current_method_ = MethodTypes::START_ARRAY;

		return { *this };
	}

	Builder& Builder::EndDict() {
		if (nodes_stack_.empty() || !nodes_stack_.back()->IsDict() || current_method_ == MethodTypes::KEY) {
			throw std::logic_error("Call EndDict for not Dict"s);
		}

		if (!nodes_stack_.empty() && nodes_stack_.back()->IsDict()) {

			if (nodes_stack_.size() == 1) {
				root_ = *nodes_stack_.back();
			}

			else if (nodes_stack_.at(nodes_stack_.size() - 2)->IsArray()) {
				std::vector<Node>& m = const_cast<Array&>(nodes_stack_.at(nodes_stack_.size() - 2)->AsArray());
				m.emplace_back(*nodes_stack_.back());
			}

			else if (nodes_stack_.at(nodes_stack_.size() - 2)->IsDict()) {
				std::map<std::string, Node>& m = const_cast<Dict&>(nodes_stack_.at(nodes_stack_.size() - 2)->AsDict());
				m.insert({ key_.back(), *nodes_stack_.back() });
			}

			{
				std::map<std::string, Node>& m = const_cast<Dict&>(nodes_stack_.back()->AsDict());
				if (!m.empty()) {
					key_.pop_back();
				}
			}

			nodes_stack_.pop_back();
			current_method_ = MethodTypes::END_DICT;
			return *this;
		}
		throw std::logic_error("Call EndDict"s);
	}

	Builder& Builder::EndArray() {
		if (nodes_stack_.empty() || !nodes_stack_.back()->IsArray() || current_method_ == MethodTypes::KEY) {
			throw std::logic_error("Call EndArray for not Array"s);
		}

		if (!nodes_stack_.empty() && nodes_stack_.back()->IsArray()) {

			if (nodes_stack_.size() == 1) {
				root_ = *nodes_stack_.back();
				nodes_stack_.pop_back();
				current_method_ = MethodTypes::END_ARRAY;
				return *this;
			}

			else if (nodes_stack_.at(nodes_stack_.size() - 2)->IsArray()) {
				std::vector<Node>& m = const_cast<Array&>(nodes_stack_.at(nodes_stack_.size() - 2)->AsArray());
				m.emplace_back(*nodes_stack_.back());
				nodes_stack_.pop_back();
				current_method_ = MethodTypes::END_ARRAY;
				return *this;
			}

			else if (nodes_stack_.at(nodes_stack_.size() - 2)->IsDict()) {
				std::map<std::string, Node>& m = const_cast<Dict&>(nodes_stack_.at(nodes_stack_.size() - 2)->AsDict());
				m.insert({ key_.back(), *nodes_stack_.back() });
				nodes_stack_.pop_back();
				current_method_ = MethodTypes::END_ARRAY;
				return *this;
			}
		}
		throw std::logic_error("Call EndArray"s);
	}

	json::Node Builder::Build() {
		if (!root_.IsNull()) {
			return root_;
		}
		else {
			throw std::logic_error("nodes_stack_.size() != 0");
		}
		current_method_ = MethodTypes::BUILD;

		return root_;
	}

	Node Builder::GetNode(Node::Value value) {
		if (std::holds_alternative<int>(value)) {
			return Node(std::get<int>(value));
		}
		else if (std::holds_alternative<double>(value)) {
			return Node(std::get<double>(value));
		}
		else if (std::holds_alternative<std::string>(value)) {
			return Node(std::get<std::string>(value));
		}
		else if (std::holds_alternative<std::nullptr_t>(value)) {
			return Node(std::get<std::nullptr_t>(value));
		}
		else if (std::holds_alternative<bool>(value)) {
			return Node(std::get<bool>(value));
		}
		else if (std::holds_alternative<Dict>(value)) {
			return Node(std::get<Dict>(value));
		}
		else if (std::holds_alternative<Array>(value)) {
			return Node(std::get<Array>(value));
		}
		return {};
	}

	void Builder::CheckStart(std::string container) {

		if (!root_.IsNull() && nodes_stack_.empty()) {
			throw std::logic_error(container + " bugs. !root_.IsNull() && nodes_stack_.empty()"s);
		}

		if (current_method_ == MethodTypes::BEFORE_START || 
		   (nodes_stack_.back()->IsDict() && current_method_ == MethodTypes::KEY) ||
		   (nodes_stack_.back()->IsArray() && (current_method_ == MethodTypes::VALUE ||
											   current_method_ == MethodTypes::START_ARRAY ||
											   current_method_ == MethodTypes::END_ARRAY ||
											   current_method_ == MethodTypes::END_DICT))) {
			return;
		}
		else {
			throw std::logic_error(container + "bugs"s);
		}
	}
}