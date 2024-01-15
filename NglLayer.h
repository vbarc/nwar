#pragma once

class NglLayer {
public:
    virtual ~NglLayer() = default;
    virtual void render() = 0;
};
