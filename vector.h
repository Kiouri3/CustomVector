#ifndef VECTOR_H
#define VECTOR_H

#include <iterator>
#include <algorithm>
#include <initializer_list>
#include <memory>
#include <stdexcept>

template <typename T>
class Vector {
 public:
  using ValueType = T;
  using Pointer = T*;
  using ConstPointer = const T*;
  using Reference = T&;
  using ConstReference = const T&;
  using SizeType = size_t;
  using Iterator = T*;
  using ConstIterator = const T*;
  using ReverseIterator = std::reverse_iterator<T*>;
  using ConstReverseIterator = std::reverse_iterator<const T*>;

  // Конструкторы
  Vector() : data_(nullptr), size_(0), capacity_(0) {}

  explicit Vector(SizeType size) : data_(nullptr), size_(0), capacity_(0) {
    if (size > 0) {
      Reserve(size);
      SizeType constructed = 0;

      try {
        for (; constructed < size; ++constructed) {
          new (data_ + constructed) T();
        }
        size_ = size;

      } catch (...) {
        for (SizeType i = 0; i < constructed; ++i) {
          data_[i].~T();
        }
        ::operator delete(data_);
        data_ = nullptr;
        capacity_ = 0;
        throw;
      }
    }
  }

  Vector(SizeType size, const T& value) : data_(nullptr), size_(0), capacity_(0) {
    if (size > 0) {
      Reserve(size);
      SizeType constructed = 0;

      try {
        for (; constructed < size; ++constructed) {
          new (data_ + constructed) T(value);
        }
        size_ = size;

      } catch (...) {
        for (SizeType i = 0; i < constructed; ++i) {
          data_[i].~T();
        }
        ::operator delete(data_);
        data_ = nullptr;
        capacity_ = 0;
        throw;
      }
    }
  }

  template <class Iterator, class = std::enable_if_t<std::is_base_of_v<std::forward_iterator_tag,typename std::iterator_traits<Iterator>::iterator_category>>>
  Vector(Iterator first, Iterator last) : data_(nullptr), size_(0), capacity_(0) {
    SizeType count = std::distance(first, last);
    if (count > 0) {
      Reserve(count);

      try {
        std::uninitialized_copy(first, last, data_);
        size_ = count;

      } catch (...) {
        ::operator delete(data_);
        data_ = nullptr;
        capacity_ = 0;
        throw;
      }
    }
  }

  Vector(const Vector& other) : data_(nullptr), size_(0), capacity_(0) {
    if (other.size_ > 0) {
      Reserve(other.size_);

      try {
        std::uninitialized_copy(other.data_, other.data_ + other.size_, data_);
        size_ = other.size_;

      } catch (...) {
        ::operator delete(data_);
        data_ = nullptr;
        capacity_ = 0;
        throw;
      }
    }
  }

  Vector(std::initializer_list<T> init) : Vector(init.begin(), init.end()) {}

  Vector(Vector&& other) : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
    other.data_ = nullptr;
    other.size_ = 0;
    other.capacity_ = 0;
  }

  // Деструктор
  ~Vector() {
    for (SizeType i = 0; i < size_; ++i) {
      data_[i].~T();
    }
    size_ = 0;
    ::operator delete(data_);
  }

  // Оператор =
  Vector& operator=(const Vector& other) {
    if (this != &other) {
      Vector tmp(other);
      Swap(tmp);
    }
    return *this;
  }

  Vector& operator=(Vector&& other) {
    if (this != &other) {
      for (SizeType i = 0; i < size_; ++i) {
        data_[i].~T();
      }
      :: operator delete(data_);
      data_ = other.data_;
      size_ = other.size_;
      capacity_ = other.capacity_;
      other.data_ = nullptr;
      other.size_ = 0;
      other.capacity_ = 0;
    }
    return *this;
  }

  // Размер
  SizeType Size() const noexcept {
    return size_;
  }

  SizeType Capacity() const noexcept {
    return capacity_;
  }
  bool Empty() const noexcept {
    return size_ == 0;
  }

  // Операторы доступа
  Reference operator[](SizeType index) noexcept {
    return data_[index];
  }
  ConstReference operator[](SizeType index) const noexcept {
    return data_[index];
  }

  Reference At(SizeType index) {
    if (index >= size_) {
      throw std::out_of_range("index out of range");
    }
    return data_[index];
  }

  ConstReference At(SizeType index) const {
    if (index >= size_) {
      throw std::out_of_range("index out of range");
    }
    return data_[index];
  }

  Reference Front() noexcept {
    return data_[0];
  }

  ConstReference Front() const noexcept {
    return data_[0];
  }

  Reference Back() noexcept {
    return data_[size_ - 1];

  }
  ConstReference Back() const noexcept {
    return data_[size_ - 1];
  }

  Pointer Data() noexcept {
    return data_;
  }

  ConstPointer Data() const noexcept {
    return data_;
  }

  void Swap(Vector& other) noexcept {
    std::swap(data_, other.data_);
    std::swap(size_, other.size_);
    std::swap(capacity_, other.capacity_);
  }

  // Управление памятью
  void Resize(SizeType new_size) {
    if (new_size < size_) {
      for (SizeType i = new_size; i < size_; ++i) {  // Удаляем лишние элементы
        data_[i].~T();
      }
      size_ = new_size;
    } else if (new_size > size_) {
      SizeType old_size = size_;
      SizeType old_capacity = capacity_;
      Pointer new_data = nullptr;

      try {
        if (new_size > capacity_) {
          SizeType new_capacity = (capacity_ == 0) ? new_size : std::max(capacity_ * 2, new_size);
          new_data = static_cast<Pointer>(::operator new(new_capacity * sizeof(T)));
          SizeType moved = 0;

          try {
            for (; moved < old_size; ++moved) {  // Перемещаем старые элементы
              new (new_data + moved) T(std::move(data_[moved]));
            }
          } catch (...) {
            for (SizeType i = 0; i < moved; ++i) {  // Если искл при перемещении
              new_data[i].~T();
            }
            throw;
          }

        } else {
          new_data = data_;
        }
        SizeType constructed = old_size;

        try {
          for (; constructed < new_size; ++constructed) {  // Создаём новые элементы
            new (new_data + constructed) T();
          }
        } catch (...) {
          for (SizeType i = old_size; i < constructed; ++i) {  // Если искл в конструкторе
            new_data[i].~T();
          }
          throw;
        }

        if (new_data != data_) {
          for (SizeType i = 0; i < old_size; ++i) {  // Уничтожаем старые элементы
            data_[i].~T();
          }
          ::operator delete(data_);
          data_ = new_data;
          capacity_ = (capacity_ == 0) ? new_size : std::max(capacity_ * 2, new_size);
        }

        size_ = new_size;
      } catch (...) {
        if (new_data && new_data != data_) {  // Откатываем при исключении
          for (SizeType i = 0; i < old_size; ++i) {
            new_data[i].~T();
          }
          ::operator delete(new_data);
        }
        size_ = old_size;
        capacity_ = old_capacity;
        throw;
      }
    }
  }

  void Resize(SizeType new_size, const T& value) {
    if (new_size < size_) {
      for (SizeType i = new_size; i < size_; ++i) {  // Удаляем лишние элементы
        data_[i].~T();
      }
      size_ = new_size;
    } else if (new_size > size_) {
      SizeType old_size = size_;
      SizeType old_capacity = capacity_;
      Pointer new_data = nullptr;

      try {
        if (new_size > capacity_) {
          SizeType new_capacity = (capacity_ == 0) ? new_size : std::max(capacity_ * 2, new_size);
          new_data = static_cast<Pointer>(::operator new(new_capacity * sizeof(T)));
          SizeType moved = 0;

          try {
            for (; moved < old_size; ++moved) {  // Перемещаем старые элементы
              new (new_data + moved) T(std::move(data_[moved]));
            }
          } catch (...) {
            for (SizeType i = 0; i < moved; ++i) {  // Если искл при перемещении
              new_data[i].~T();
            }
            throw;
          }

        } else {
          new_data = data_;
        }
        SizeType constructed = old_size;

        try {
          for (; constructed < new_size; ++constructed) {  // Создаём новые элементы
            new (new_data + constructed) T(value);
          }
        } catch (...) {
          for (SizeType i = old_size; i < constructed; ++i) {  // Если искл в конструкторе
            new_data[i].~T();
          }
          throw;
        }

        if (new_data != data_) {
          for (SizeType i = 0; i < old_size; ++i) {  // Уничтожаем старые элементы
            data_[i].~T();
          }
          ::operator delete(data_);
          data_ = new_data;
          capacity_ = (capacity_ == 0) ? new_size : std::max(capacity_ * 2, new_size);
        }

        size_ = new_size;
      } catch (...) {
        if (new_data && new_data != data_) {  // Откатываем при исключении
          for (SizeType i = 0; i < old_size; ++i) {
            new_data[i].~T();
          }
          ::operator delete(new_data);
        }
        size_ = old_size;
        capacity_ = old_capacity;
        throw;
      }
    }
  }

  void Reserve(SizeType new_cap) {
    if (new_cap > capacity_) {
      auto new_data = static_cast<Pointer>(::operator new(new_cap * sizeof(T)));
      SizeType new_size = 0;

      try {
        for (; new_size < size_; ++new_size) {  // Копируем
          new (new_data + new_size) T(std::move(data_[new_size]));
        }
      } catch (...) {
        for (SizeType i = 0; i < new_size; ++i) {
          new_data[i].~T();
        }
        ::operator delete(new_data);
        throw;
      }

      for (SizeType i = 0; i < size_; ++i) {
        data_[i].~T();
      }
      ::operator delete(data_);
      data_ = new_data;
      capacity_ = new_cap;
    }
  }

  void ShrinkToFit() {
    if (size_ < capacity_) {
      if (size_ == 0) {
        ::operator delete(data_);
        data_ = nullptr;
        capacity_ = 0;
      } else {
        auto new_data = static_cast<Pointer>(::operator new(size_ * sizeof(T)));
        SizeType new_size = 0;
        try {
          for (; new_size < size_; ++new_size) {
            new (new_data + new_size) T(std::move(data_[new_size]));
          }
        } catch (...) {
          for (SizeType i = 0; i < new_size; ++i) {
            new_data[i].~T();
          }
          ::operator delete(new_data);
          throw;
        }

        for (SizeType i = 0; i < size_; ++i) {
          data_[i].~T();
        }
        ::operator delete(data_);
        data_ = new_data;
        capacity_ = size_;
      }
    }
  }

  void Clear() noexcept {
    for (SizeType i = 0; i < size_; ++i) {
      data_[i].~T();
    }
    size_ = 0;
  }

  // Добавление элементов
  void PushBack(const T& value) {
    SizeType old_size = size_;
    SizeType old_capacity = capacity_;
    Pointer new_data = nullptr;

    try {
      if (size_ == capacity_) {
        SizeType new_capacity = (capacity_ == 0) ? 1 : capacity_ * 2;
        new_data = static_cast<Pointer>(::operator new(new_capacity * sizeof(T)));

        for (SizeType i = 0; i < old_size; ++i) {  // Копируем элементы
          new (new_data + i) T(std::move(data_[i]));
        }
      } else {
        new_data = data_;  // Если память не нужно выделять
      }

      new (new_data + old_size) T(value);
      if (new_data != data_) {
        for (SizeType i = 0; i < old_size; ++i) {  // освобождаем старую память
          data_[i].~T();
        }
        ::operator delete(data_);
        data_ = new_data;
        capacity_ = (capacity_ == 0) ? 1 : capacity_ * 2;
      }

      size_ = old_size + 1;

    } catch (...) {
      if (new_data && new_data != data_) {  // Если возникло исключение, откатываемся
        for (SizeType i = 0; i < old_size; ++i) {
          new_data[i].~T();
        }
        ::operator delete(new_data);
      }
      throw;
    }
  }

  void PushBack(T&& value) {
    SizeType old_size = size_;
    SizeType old_capacity = capacity_;
    Pointer new_data = nullptr;

    try {
      if (size_ == capacity_) {
        SizeType new_capacity = (capacity_ == 0) ? 1 : capacity_ * 2;
        new_data = static_cast<Pointer>(::operator new(new_capacity * sizeof(T)));

        for (SizeType i = 0; i < old_size; ++i) {  // Копируем элементы
          new (new_data + i) T(std::move(data_[i]));
        }
      } else {
        new_data = data_;  // Если память не нужно выделять
      }

      new (new_data + old_size) T(std::move(value));
      if (new_data != data_) {
        for (SizeType i = 0; i < old_size; ++i) {  // освобождаем старую память
          data_[i].~T();
        }
        ::operator delete(data_);
        data_ = new_data;
        capacity_ = (capacity_ == 0) ? 1 : capacity_ * 2;
      }

      size_ = old_size + 1;

    } catch (...) {
      if (new_data && new_data != data_) {  // Если возникло исключение, откатываемся
        for (SizeType i = 0; i < old_size; ++i) {
          new_data[i].~T();
        }
        ::operator delete(new_data);
      }
      throw;
    }
  }

  void PopBack() {
    if (size_ > 0) {
      --size_;
      (data_ + size_)->~T();
    }
  }

  // Итераторы
  Iterator begin() noexcept {  // NOLINT
    return data_;
  }

  ConstIterator begin() const noexcept {  // NOLINT
    return data_;
  }

  Iterator end() noexcept {  // NOLINT
    return data_ + size_;
  }

  ConstIterator end() const noexcept {  // NOLINT
    return data_ + size_;
  }

  ConstIterator cbegin() const noexcept {  // NOLINT
    return data_;
  }

  ConstIterator cend() const noexcept {  // NOLINT
    return data_ + size_;
  }

  ReverseIterator rbegin() noexcept {  // NOLINT
    return ReverseIterator(data_ + size_);
  }

  ConstReverseIterator rbegin() const noexcept {  // NOLINT
    return ConstReverseIterator(data_ + size_);
  }

  ReverseIterator rend() noexcept {  // NOLINT
    return ReverseIterator(data_);
  }

  ConstReverseIterator rend() const noexcept {  // NOLINT
    return ConstReverseIterator(data_);
  }

  ConstReverseIterator crbegin() const noexcept {  // NOLINT
    return ConstReverseIterator(data_ + size_);
  }

  ConstReverseIterator crend() const noexcept {  // NOLINT
    return ConstReverseIterator(data_);
  }


private:
  Pointer data_;
  SizeType size_;
  SizeType capacity_;
};

template <typename T>
bool operator==(const Vector<T>& lhs, const Vector<T>& rhs) {
  if (lhs.Size() != rhs.Size()) {
    return false;
  }
  return std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename T>
bool operator!=(const Vector<T>& lhs, const Vector<T>& rhs) {
  return !(lhs == rhs);
}

template <typename T>
bool operator<(const Vector<T>& lhs, const Vector<T>& rhs) {
  return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename T>
bool operator<=(const Vector<T>& lhs, const Vector<T>& rhs) {
  return !(rhs < lhs);
}

template <typename T>
bool operator>(const Vector<T>& lhs, const Vector<T>& rhs) {
  return rhs < lhs;
}

template <typename T>
bool operator>=(const Vector<T>& lhs, const Vector<T>& rhs) {
  return !(lhs < rhs);
}

#endif