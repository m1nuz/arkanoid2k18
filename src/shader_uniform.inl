namespace video {
    static auto set_value(const resources::shader_t &sh, const std::string_view name, const mat4 &matrix) -> void {
        const auto it = sh.uniforms.find(name.data());
        if (it == sh.uniforms.end())
            return;

        glUniformMatrix4fv(it->second.location, 1, GL_FALSE, glm::value_ptr(matrix));
    }

    static auto set_value(const resources::shader_t &sh, const std::string_view name, const int v) {
        const auto it = sh.uniforms.find(name.data());
        if (it == sh.uniforms.end())
            return;

        glUniform1i(it->second.location, v);
    }

    static auto set_value(const resources::shader_t &sh, const std::string_view name, const float v) {
        const auto it = sh.uniforms.find(name.data());
        if (it == sh.uniforms.end())
            return;

        glUniform1f(it->second.location, v);
    }

    static auto set_value(const resources::shader_t &sh, const std::string_view name, const vec2 &v) {
        const auto it = sh.uniforms.find(name.data());
        if (it == sh.uniforms.end())
            return;

        glUniform2f(it->second.location, v.x, v.y);
    }

    static auto set_value(const resources::shader_t &sh, const std::string_view name, const vec3 &v) {
        const auto it = sh.uniforms.find(name.data());
        if (it == sh.uniforms.end())
            return;

        glUniform3f(it->second.location, v.x, v.y, v.z);
    }

    static auto set_value(const resources::shader_t &sh, const std::string_view name, const vec4 &v) {
        const auto it = sh.uniforms.find(name.data());
        if (it == sh.uniforms.end())
            return;

        glUniform4f(it->second.location, v.x, v.y, v.z, v.w);
    }

    template <std::size_t N>
    auto set_value(const resources::shader_t &sh, const std::string_view name, const int (&values)[N]) -> void {
        const auto it = std::find_if(sh.uniforms.begin(), sh.uniforms.end(), [name] (const auto& u) {
            return u.first.find(name.data()) != std::string::npos;
        });

        if (it == sh.uniforms.end())
            return;

        glUniform1iv(it->second.location, N, &values[0]);
    }

    template <std::size_t N>
    auto set_value(const resources::shader_t &sh, const std::string_view name, const float (&values)[N]) -> void {
        const auto it = std::find_if(sh.uniforms.begin(), sh.uniforms.end(), [name] (const auto& u) {
            return u.first.find(name.data()) != std::string::npos;
        });

        if (it == sh.uniforms.end())
            return;

        glUniform1fv(it->second.location, N, &values[0]);
    }

    static auto set_value(const resources::shader_t &sh, const std::string_view name, const std::vector<vec2> &values) -> void {
        const auto it = std::find_if(sh.uniforms.begin(), sh.uniforms.end(), [name] (const auto& u) {
            return u.first.find(name.data()) != std::string::npos;
        });

        if (it == sh.uniforms.end())
            return;

        glUniform2fv(it->second.location, values.size(), &values[0].x);
    }
} // namespace video
