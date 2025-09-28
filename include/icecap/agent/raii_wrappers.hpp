#ifndef ICECAP_AGENT_RAII_WRAPPERS_HPP
#define ICECAP_AGENT_RAII_WRAPPERS_HPP

#include <windows.h>
#include <d3d9.h>
#include <memory>

namespace icecap::agent::raii {

/**
 * RAII wrapper for IDirect3D9 objects
 */
class D3D9Device {
public:
    explicit D3D9Device(IDirect3D9* d3d) : m_d3d(d3d) {}

    ~D3D9Device() {
        if (m_d3d) {
            m_d3d->Release();
            m_d3d = nullptr;
        }
    }

    // Non-copyable, movable
    D3D9Device(const D3D9Device&) = delete;
    D3D9Device& operator=(const D3D9Device&) = delete;

    D3D9Device(D3D9Device&& other) noexcept : m_d3d(other.m_d3d) {
        other.m_d3d = nullptr;
    }

    D3D9Device& operator=(D3D9Device&& other) noexcept {
        if (this != &other) {
            if (m_d3d) {
                m_d3d->Release();
            }
            m_d3d = other.m_d3d;
            other.m_d3d = nullptr;
        }
        return *this;
    }

    IDirect3D9* get() const { return m_d3d; }
    IDirect3D9* operator->() const { return m_d3d; }
    explicit operator bool() const { return m_d3d != nullptr; }

private:
    IDirect3D9* m_d3d;
};

/**
 * RAII wrapper for IDirect3DDevice9 objects
 */
class D3D9DeviceWrapper {
public:
    explicit D3D9DeviceWrapper(IDirect3DDevice9* device) : m_device(device) {}

    ~D3D9DeviceWrapper() {
        if (m_device) {
            m_device->Release();
            m_device = nullptr;
        }
    }

    // Non-copyable, movable
    D3D9DeviceWrapper(const D3D9DeviceWrapper&) = delete;
    D3D9DeviceWrapper& operator=(const D3D9DeviceWrapper&) = delete;

    D3D9DeviceWrapper(D3D9DeviceWrapper&& other) noexcept : m_device(other.m_device) {
        other.m_device = nullptr;
    }

    D3D9DeviceWrapper& operator=(D3D9DeviceWrapper&& other) noexcept {
        if (this != &other) {
            if (m_device) {
                m_device->Release();
            }
            m_device = other.m_device;
            other.m_device = nullptr;
        }
        return *this;
    }

    IDirect3DDevice9* get() const { return m_device; }
    IDirect3DDevice9* operator->() const { return m_device; }
    explicit operator bool() const { return m_device != nullptr; }

private:
    IDirect3DDevice9* m_device;
};

/**
 * RAII wrapper for Windows thread handles
 */
class ThreadHandle {
public:
    explicit ThreadHandle(HANDLE handle) : m_handle(handle) {}

    ~ThreadHandle() {
        if (m_handle && m_handle != INVALID_HANDLE_VALUE) {
            CloseHandle(m_handle);
            m_handle = nullptr;
        }
    }

    // Non-copyable, movable
    ThreadHandle(const ThreadHandle&) = delete;
    ThreadHandle& operator=(const ThreadHandle&) = delete;

    ThreadHandle(ThreadHandle&& other) noexcept : m_handle(other.m_handle) {
        other.m_handle = nullptr;
    }

    ThreadHandle& operator=(ThreadHandle&& other) noexcept {
        if (this != &other) {
            if (m_handle && m_handle != INVALID_HANDLE_VALUE) {
                CloseHandle(m_handle);
            }
            m_handle = other.m_handle;
            other.m_handle = nullptr;
        }
        return *this;
    }

    HANDLE get() const { return m_handle; }
    explicit operator bool() const { return m_handle != nullptr && m_handle != INVALID_HANDLE_VALUE; }

    // Wait for thread with timeout
    DWORD wait(DWORD timeout = INFINITE) const {
        if (!m_handle || m_handle == INVALID_HANDLE_VALUE) {
            return WAIT_FAILED;
        }
        return WaitForSingleObject(m_handle, timeout);
    }

private:
    HANDLE m_handle;
};

} // namespace icecap::agent::raii

#endif // ICECAP_AGENT_RAII_WRAPPERS_HPP