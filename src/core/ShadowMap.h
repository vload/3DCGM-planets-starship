#pragma once


#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()

#include <glad/glad.h>
DISABLE_WARNINGS_POP()
#include "core/config.h"

class ShadowMap {
   private:
    Config& config;
    GLuint fbo;
    GLuint depth_texture;
    const int shadow_map_size = 2048;

   public:
    ShadowMap(Config& config) : config(config), shadow_map_size(config.shadow_map_size) {
        // Create the framebuffer
        glGenFramebuffers(1, &fbo);

        glGenTextures(1, &depth_texture);
        glBindTexture(GL_TEXTURE_2D, depth_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadow_map_size,
                     shadow_map_size, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

        // Set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        float borderColor[] = {1.0, 1.0, 1.0, 1.0};
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                               GL_TEXTURE_2D, depth_texture, 0);

        // No color buffer is drawn or read
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    ~ShadowMap(){
        glDeleteFramebuffers(1, &fbo);
        glDeleteTextures(1, &depth_texture);
    }

    void bind_for_writing(){
        glViewport(0, 0, shadow_map_size, shadow_map_size);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        glClear(GL_DEPTH_BUFFER_BIT);
    }

    void bind_for_reading(GLenum texture_unit){
        glActiveTexture(texture_unit);
        glBindTexture(GL_TEXTURE_2D, depth_texture);
    }

};
