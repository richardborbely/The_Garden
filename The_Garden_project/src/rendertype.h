#pragma once

// RenderType object that stores the type of render for a mesh and optional visual effects
class RenderType
{
  int type;
  float reflectionAmount;
  float refractionAmount;
  float fresnelIntensity;

public:
  RenderType();
  RenderType(int _type, float _reflectionAmount, float _refractionAmount, float _fresnelIntensity);

  int getRenderType();
  float getReflectionAmount();
  float getRefractionAmount();
  float getFresnelIntensity();
};
