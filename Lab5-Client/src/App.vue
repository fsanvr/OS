<template>
  <div id="app">
    <!-- Вставляем компонент TemperatureDashboard -->
    <TemperatureDashboard />
  </div>
</template>

<script>
// Импортируем компонент TemperatureDashboard
import TemperatureDashboard from './components/TemperatureDashboard.vue'

export default {
  name: 'App',
  components: {
    // Регистрируем компонент для использования в шаблоне
    TemperatureDashboard
  }
}
</script>

<style>
#app {
  font-family: Avenir, Helvetica, Arial, sans-serif;
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;
  text-align: center;
  color: #2c3e50;
  margin-top: 60px;
}
</style>